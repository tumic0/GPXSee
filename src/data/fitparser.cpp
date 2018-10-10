#include <QtEndian>
#include "common/staticassert.h"
#include "fitparser.h"


#define FIT_MAGIC 0x5449462E // .FIT

#define RECORD_MESSAGE  20
#define EVENT_MESSAGE   21
#define TIMESTAMP_FIELD 253

class Event {
public:
	Event() : id(0), type(0), data(0) {}

	quint8 id;
	quint8 type;
	quint32 data;
};

struct FileHeader {
	quint8 headerSize;
	quint8 protocolVersion;
	quint16 profileVersion;
	quint32 dataSize;
	quint32 magic;
};

struct FITParser::Field {
	quint8 id;
	quint8 size;
	quint8 type;
};

class FITParser::MessageDefinition {
public:
	MessageDefinition() : endian(0), globalId(0), numFields(0), fields(0),
	  numDevFields(0), devFields(0) {}
	~MessageDefinition() {delete[] fields; delete[] devFields;}

	quint8 endian;
	quint16 globalId;
	quint8 numFields;
	Field *fields;
	quint8 numDevFields;
	Field *devFields;
};

class FITParser::CTX {
public:
	CTX(QFile *file) : file(file), len(0), endian(0), timestamp(0),
		lastWrite(0), ratio(NAN) {}

	QFile *file;
	quint32 len;
	quint8 endian;
	quint32 timestamp, lastWrite;
	MessageDefinition defs[16];
	qreal ratio;
	Trackpoint trackpoint;
	TrackData track;
};


bool FITParser::readData(QFile *file, char *data, size_t size)
{
	qint64 n;

	n = file->read(data, size);
	if (n < 0) {
		_errorString = "I/O error";
		return false;
	} else if ((size_t)n < size) {
		_errorString = "Premature end of data";
		return false;
	}

	return true;
}

template<class T> bool FITParser::readValue(CTX &ctx, T &val)
{
	T data;

	if (!readData(ctx.file, (char*)&data, sizeof(T)))
		return false;

	ctx.len -= sizeof(T);

	if (sizeof(T) > 1) {
		if (ctx.endian)
			val = qFromBigEndian(data);
		else
			val = qFromLittleEndian(data);
	} else
		val = data;

	return true;
}

bool FITParser::skipValue(CTX &ctx, quint8 size)
{
	ctx.len -= size;
	return ctx.file->seek(ctx.file->pos() + size);
}

bool FITParser::parseDefinitionMessage(CTX &ctx, quint8 header)
{
	int local_id = header & 0x0f;
	MessageDefinition *def = &(ctx.defs[local_id]);
	quint8 i;


	if (def->fields) {
		delete[] def->fields;
		def->fields = 0;
	}
	if (def->devFields) {
		delete[] def->devFields;
		def->devFields = 0;
	}

	// reserved/unused
	if (!readValue(ctx, i))
		return false;

	// endianness
	if (!readValue(ctx, def->endian))
		return false;
	if (def->endian > 1) {
		_errorString = "Bad endian field";
		return false;
	}
	ctx.endian = def->endian;

	// global message number
	if (!readValue(ctx, def->globalId))
		return false;

	// number of records
	if (!readValue(ctx, def->numFields))
		return false;

	// definition records
	def->fields = new Field[def->numFields];
	for (i = 0; i < def->numFields; i++) {
		STATIC_ASSERT(sizeof(def->fields[i]) == 3);
		if (!readData(ctx.file, (char*)&(def->fields[i]),
		  sizeof(def->fields[i])))
			return false;
		ctx.len -= sizeof(def->fields[i]);
	}

	// developer definition records
	if (header & 0x20) {
		if (!readValue(ctx, def->numDevFields))
			return false;

		def->devFields = new Field[def->numDevFields];
		for (i = 0; i < def->numDevFields; i++) {
			STATIC_ASSERT(sizeof(def->devFields[i]) == 3);
			if (!readData(ctx.file, (char*)&(def->devFields[i]),
			  sizeof(def->devFields[i])))
				return false;
			ctx.len -= sizeof(def->devFields[i]);
		}
	} else
		def->numDevFields = 0;

	return true;
}

bool FITParser::readField(CTX &ctx, Field *field, quint32 &val)
{
	quint8 v8 = (quint8)-1;
	quint16 v16 = (quint16)-1;
	bool ret;

	val = (quint32)-1;

	switch (field->type) {
		case 0: // enum
		case 1: // sint8
		case 2: // uint8
			if (field->size == 1) {
				ret = readValue(ctx, v8);
				val = v8;
			} else
				ret = skipValue(ctx, field->size);
			break;
		case 0x83: // sint16
		case 0x84: // uint16
			if (field->size == 2) {
				ret = readValue(ctx, v16);
				val = v16;
			} else
				ret = skipValue(ctx, field->size);
			break;
		case 0x85: // sint32
		case 0x86: // uint32
			if (field->size == 4)
				ret = readValue(ctx, val);
			else
				ret = skipValue(ctx, field->size);
			break;
		default:
			ret = skipValue(ctx, field->size);
			break;
	}

	return ret;
}

bool FITParser::parseData(CTX &ctx, const MessageDefinition *def)
{
	Field *field;
	Event event;
	quint32 val;


	if (!def->fields && !def->devFields) {
		_errorString = "Undefined data message";
		return false;
	}

	ctx.endian = def->endian;

	for (int i = 0; i < def->numFields; i++) {
		field = &def->fields[i];
		if (!readField(ctx, field, val))
			return false;

		if (field->id == TIMESTAMP_FIELD)
			ctx.timestamp = val;
		else if (def->globalId == RECORD_MESSAGE) {
			switch (field->id) {
				case 0:
					if (val != 0x7fffffff)
						ctx.trackpoint.rcoordinates().setLat(
						  ((qint32)val / (double)0x7fffffff) * 180);
					break;
				case 1:
					if (val != 0x7fffffff)
						ctx.trackpoint.rcoordinates().setLon(
						  ((qint32)val / (double)0x7fffffff) * 180);
					break;
				case 2:
					if (val != 0xffff)
						ctx.trackpoint.setElevation((val / 5.0) - 500);
					break;
				case 3:
					if (val != 0xff)
						ctx.trackpoint.setHeartRate(val);
					break;
				case 4:
					if (val != 0xff)
						ctx.trackpoint.setCadence(val);
					break;
				case 6:
					if (val != 0xffff)
						ctx.trackpoint.setSpeed(val / 1000.0f);
					break;
				case 7:
					if (val != 0xffff)
						ctx.trackpoint.setPower(val);
					break;
				case 13:
					if (val != 0x7f)
						ctx.trackpoint.setTemperature((qint8)val);
					break;
				default:
					break;

			}
		} else if (def->globalId == EVENT_MESSAGE) {
			switch (field->id) {
				case 0:
					event.id = val;
					break;
				case 1:
					event.type = val;
					break;
				case 3:
					event.data = val;
					break;
			}
		}
	}

	for (int i = 0; i < def->numDevFields; i++) {
		field = &def->devFields[i];
		if (!readField(ctx, field, val))
			return false;
	}


	if (def->globalId == EVENT_MESSAGE) {
		if ((event.id == 42 || event.id == 43)  && event.type == 3) {
			quint32 front = ((event.data & 0xFF000000) >> 24);
			quint32 rear = ((event.data & 0x0000FF00) >> 8);
			ctx.ratio = ((qreal)front / (qreal)rear);
		}
	} else if (def->globalId == RECORD_MESSAGE) {
		if (ctx.timestamp > ctx.lastWrite
		  && ctx.trackpoint.coordinates().isValid()) {
			ctx.trackpoint.setTimestamp(QDateTime::fromTime_t(ctx.timestamp
			  + 631065600));
			ctx.trackpoint.setRatio(ctx.ratio);
			ctx.track.append(ctx.trackpoint);
			ctx.trackpoint = Trackpoint();
			ctx.lastWrite = ctx.timestamp;
		}
	}

	return true;
}

bool FITParser::parseDataMessage(CTX &ctx, quint8 header)
{
	int local_id = header & 0xf;
	MessageDefinition *def = &(ctx.defs[local_id]);
	return parseData(ctx, def);
}

bool FITParser::parseCompressedMessage(CTX &ctx, quint8 header)
{
	int local_id = (header >> 5) & 3;
	MessageDefinition *def = &(ctx.defs[local_id]);
	ctx.timestamp += header & 0x1f;
	return parseData(ctx, def);
}

bool FITParser::parseRecord(CTX &ctx)
{
	quint8 header;

	if (!readValue(ctx, header))
		return false;

	if (header & 0x80)
		return parseCompressedMessage(ctx, header);
	else if (header & 0x40)
		return parseDefinitionMessage(ctx, header);
	else
		return parseDataMessage(ctx, header);
}

bool FITParser::parseHeader(CTX &ctx)
{
	FileHeader hdr;
	quint16 crc;
	qint64 len;

	STATIC_ASSERT(sizeof(hdr) == 12);
	len = ctx.file->read((char*)&hdr, sizeof(hdr));
	if (len < 0) {
		_errorString = "I/O error";
		return false;
	} else if ((size_t)len < sizeof(hdr)
	  || hdr.magic != qToLittleEndian((quint32)FIT_MAGIC)) {
		_errorString = "Not a FIT file";
		return false;
	}

	ctx.len = qFromLittleEndian(hdr.dataSize);

	if (hdr.headerSize > sizeof(hdr))
		if (!readData(ctx.file, (char *)&crc, sizeof(crc)))
			return false;

	return true;
}

bool FITParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(waypoints);
	CTX ctx(file);


	if (!parseHeader(ctx))
		return false;

	while (ctx.len)
		if (!parseRecord(ctx))
			return false;

	tracks.append(ctx.track);

	return true;
}
