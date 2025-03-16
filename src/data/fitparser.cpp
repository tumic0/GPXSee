#include <QtEndian>
#include <QTimeZone>
#include "GUI/format.h"
#include "fitparser.h"

#define FIT_MAGIC 0x5449462E // .FIT

#define LAP         19
#define RECORD      20
#define EVENT       21
#define LOCATION    29
#define COURSEPOINT 32

#define TIMESTAMP   253

static QMap<int, QString> coursePointSymbolsInit()
{
	QMap<int, QString> map;

	map.insert(1, "Summit");
	map.insert(2, "Valley");
	map.insert(3, "Water");
	map.insert(4, "Food");
	map.insert(5, "Danger");
	map.insert(6, "Left");
	map.insert(7, "Right");
	map.insert(8, "Straight");
	map.insert(9, "First Aid");
	map.insert(10, "Fourth Category");
	map.insert(11, "Third Category");
	map.insert(12, "Second Category");
	map.insert(13, "First Category");
	map.insert(14, "Hors Category");
	map.insert(15, "Sprint");
	map.insert(16, "Left Fork");
	map.insert(17, "Right Fork");
	map.insert(18, "Middle Fork");
	map.insert(19, "Slight Left");
	map.insert(20, "Sharp Left");
	map.insert(21, "Slight Right");
	map.insert(22, "Sharp Right");
	map.insert(23, "U-Turn");
	map.insert(24, "Segment Start");
	map.insert(25, "Segment End");
	map.insert(27, "Campground");
	map.insert(28, "Aid Station");
	map.insert(29, "Rest Area");
	map.insert(30, "General Distance");
	map.insert(31, "Service");
	map.insert(32, "Energy Gel");
	map.insert(33, "Sports Drink");
	map.insert(34, "Mile Marker");
	map.insert(35, "Checkpoint");
	map.insert(36, "Shelter");
	map.insert(37, "Meeting Spot");
	map.insert(38, "Overlook");
	map.insert(39, "Toilet");
	map.insert(40, "Shower");
	map.insert(41, "Gear");
	map.insert(42, "Sharp Curve");
	map.insert(43, "Steep Incline");
	map.insert(44, "Tunnel");
	map.insert(45, "Bridge");
	map.insert(46, "Obstacle");
	map.insert(47, "Crossing");
	map.insert(48, "Store");
	map.insert(49, "Transition");
	map.insert(50, "Navaid");
	map.insert(51, "Transport");
	map.insert(52, "Alert");
	map.insert(53, "Info");

	return map;
}

static QMap<int, QString> locationPointSymbolsInit()
{
	QMap<int, QString> map;

	/* The location symbols are a typical GARMIN mess. Every GPS unit
	   has probably its own list, so we only add a few generic icons seen
	   "in the wild" most often. */
	map.insert(94, "Flag, Blue");
	map.insert(95, "Flag, Green");
	map.insert(96, "Flag, Red");

	return map;
}

static QMap<int, QString> coursePointSymbols = coursePointSymbolsInit();
static QMap<int, QString> locationPointSymbols = locationPointSymbolsInit();


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
	if (!readData(ctx.file, (char*)&val, sizeof(T)))
		return false;

	ctx.len -= sizeof(T);
	if (sizeof(T) > 1)
		val = (ctx.endian) ? qFromBigEndian(val) : qFromLittleEndian(val);

	return true;
}

bool FITParser::skipValue(CTX &ctx, quint8 size)
{
	ctx.len -= size;
	return ctx.file->seek(ctx.file->pos() + size);
}

bool FITParser::parseDefinitionMessage(CTX &ctx, quint8 header)
{
	MessageDefinition *def = &(ctx.defs[header & 0x0f]);
	quint8 numFields;

	def->fields.clear();
	def->devFields.clear();

	// reserved/unused
	if (!skipValue(ctx, 1))
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

	// definition records
	if (!readValue(ctx, numFields))
		return false;

	def->fields.resize(numFields);
	for (int i = 0; i < def->fields.size(); i++) {
		if (!readData(ctx.file, (char*)&(def->fields[i]), sizeof(Field)))
			return false;
		ctx.len -= sizeof(Field);
	}

	// developer definition records
	if (header & 0x20) {
		if (!readValue(ctx, numFields))
			return false;

		def->devFields.resize(numFields);
		for (int i = 0; i < def->devFields.size(); i++) {
			if (!readData(ctx.file, (char*)&(def->devFields[i]), sizeof(Field)))
				return false;
			ctx.len -= sizeof(Field);
		}
	}

	return true;
}

bool FITParser::readField(CTX &ctx, const Field *field, QVariant &val,
  bool &valid)
{
	bool ret;

#define VAL(type, inval) \
	{type var; \
	if (field->size == sizeof(var)) { \
		ret = readValue(ctx, var); \
		val = var; \
		valid = (var != (inval)); \
	} else { \
		ret = skipValue(ctx, field->size); \
		valid = false; \
	}}

	switch (field->type) {
		case 1: // sint8
			VAL(qint8, 0x7fU);
			break;
		case 2: // uint8
		case 0: // enum
			VAL(quint8, 0xffU);
			break;
		case 3:
		case 0x83: // sint16
			VAL(qint16, 0x7fffU);
			break;
		case 4:
		case 0x84: // uint16
			VAL(quint16, 0xffffU);
			break;
		case 5:
		case 0x85: // sint32
			VAL(qint32, 0x7fffffffU);
			break;
		case 6:
		case 0x86: // uint32
			VAL(quint32, 0xffffffffU);
			break;
		case 7: // UTF8 nul terminated string
			{QByteArray ba(ctx.file->read(field->size));
			ctx.len -= field->size;
			ret = (ba.size() == field->size);
			val = ret ? QString(ba.left(ba.indexOf('\0'))) : QString();
			valid = (!ba.isEmpty() && ba.at(0) != 0);}
			break;
		default:
			ret = skipValue(ctx, field->size);
			valid = false;
			break;
	}

	return ret;
}

bool FITParser::parseData(CTX &ctx, const MessageDefinition *def)
{
	QVariant val;
	bool valid;
	Event event;
	Waypoint waypoint;
	int trigger = -1;

	if (!def->fields.size() && !def->devFields.size()) {
		_errorString = "Undefined data message";
		return false;
	}

	ctx.endian = def->endian;

	for (int i = 0; i < def->fields.size(); i++) {
		const Field *field = &def->fields.at(i);
		if (!readField(ctx, field, val, valid))
			return false;
		if (!valid)
			continue;

		if (field->id == TIMESTAMP)
			ctx.timestamp = val.toUInt();
		else if (def->globalId == RECORD) {
			switch (field->id) {
				case 0:
					ctx.trackpoint.rcoordinates().setLat(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 1:
					ctx.trackpoint.rcoordinates().setLon(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 2:
					ctx.trackpoint.setElevation((val.toUInt() / 5.0) - 500);
					break;
				case 3:
					ctx.trackpoint.setHeartRate(val.toUInt());
					break;
				case 4:
					ctx.trackpoint.setCadence(val.toUInt());
					break;
				case 6:
					ctx.trackpoint.setSpeed(val.toUInt() / 1000.0f);
					break;
				case 7:
					ctx.trackpoint.setPower(val.toUInt());
					break;
				case 13:
					ctx.trackpoint.setTemperature(val.toInt());
					break;
				case 73:
					ctx.trackpoint.setSpeed(val.toUInt() / 1000.0f);
					break;
				case 78:
					ctx.trackpoint.setElevation((val.toUInt() / 5.0) - 500);
					break;
			}
		} else if (def->globalId == EVENT) {
			switch (field->id) {
				case 0:
					event.id = val.toUInt();
					break;
				case 1:
					event.type = val.toUInt();
					break;
				case 3:
					event.data = val.toUInt();
					break;
			}
		} else if (def->globalId == COURSEPOINT) {
			switch (field->id) {
				case 1:
					waypoint.setTimestamp(QDateTime::fromSecsSinceEpoch(
					  val.toUInt() + 631065600, QTimeZone::utc()));
					break;
				case 2:
					waypoint.rcoordinates().setLat(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 3:
					waypoint.rcoordinates().setLon(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 5:
					waypoint.setSymbol(coursePointSymbols.value(val.toUInt()));
					break;
				case 6:
					waypoint.setName(val.toString());
					break;
			}
		} else if (def->globalId == LOCATION) {
			switch (field->id) {
				case 0:
					waypoint.setName(val.toString());
					break;
				case 1:
					waypoint.rcoordinates().setLat(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 2:
					waypoint.rcoordinates().setLon(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 3:
					waypoint.setSymbol(locationPointSymbols.value(val.toUInt()));
					break;
				case 4:
					waypoint.setElevation((val.toUInt() / 5.0) - 500);
					break;
				case 6:
					waypoint.setDescription(val.toString());
					break;
			}
		} else if (def->globalId == LAP) {
			switch (field->id) {
				case 5:
					waypoint.rcoordinates().setLat(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 6:
					waypoint.rcoordinates().setLon(
					  (val.toInt() / (double)0x7fffffff) * 180);
					break;
				case 7:
					waypoint.setDescription(Format::timeSpan(val.toUInt() / 1000));
					break;
				case 24:
					trigger = val.toInt();
					break;
			}
		}
	}

	for (int i = 0; i < def->devFields.size(); i++)
		if (!readField(ctx, &def->devFields.at(i), val, valid))
			return false;

	if (def->globalId == EVENT) {
		if ((event.id == 42 || event.id == 43) && event.type == 3) {
			quint32 front = ((event.data & 0xFF000000) >> 24);
			quint32 rear = ((event.data & 0x0000FF00) >> 8);
			ctx.ratio = ((qreal)front / (qreal)rear);
		}
	} else if (def->globalId == RECORD) {
		if (ctx.trackpoint.coordinates().isValid()) {
			ctx.trackpoint.setTimestamp(QDateTime::fromSecsSinceEpoch(
			  ctx.timestamp + 631065600, QTimeZone::utc()));
			ctx.trackpoint.setRatio(ctx.ratio);
			ctx.segment.append(ctx.trackpoint);
			ctx.trackpoint = Trackpoint();
		}
	} else if (def->globalId == COURSEPOINT) {
		if (waypoint.coordinates().isValid())
			ctx.waypoints.append(waypoint);
	} else if (def->globalId == LOCATION) {
		if (waypoint.coordinates().isValid()) {
			waypoint.setTimestamp(QDateTime::fromSecsSinceEpoch(ctx.timestamp
			  + 631065600, QTimeZone::utc()));
			ctx.waypoints.append(waypoint);
		}
	} else if (def->globalId == LAP && trigger >= 0) {
		if (waypoint.coordinates().isValid()) {
			if (trigger == 7)
				waypoint.setName("Finish");
			else
				waypoint.setName("Lap " + QString::number(++ctx.laps));
			waypoint.setTimestamp(QDateTime::fromSecsSinceEpoch(ctx.timestamp
			  + 631065600, QTimeZone::utc()));
			if (trigger != 7 || ctx.laps > 1)
				ctx.waypoints.append(waypoint);
		}
	}

	return true;
}

bool FITParser::parseDataMessage(CTX &ctx, quint8 header)
{
	int localId = header & 0xf;
	MessageDefinition *def = &(ctx.defs[localId]);
	return parseData(ctx, def);
}

bool FITParser::parseCompressedMessage(CTX &ctx, quint8 header)
{
	int localId = (header >> 5) & 3;
	MessageDefinition *def = &(ctx.defs[localId]);
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
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	CTX ctx(file, waypoints);

	if (!parseHeader(ctx))
		return false;

	while (ctx.len)
		if (!parseRecord(ctx))
			return false;

	tracks.append(ctx.segment);
	tracks.last().setFile(file->fileName());

	return true;
}
