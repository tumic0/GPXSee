#ifndef FITPARSER_H
#define FITPARSER_H

#include "parser.h"

class FITParser : public Parser
{
public:
	FITParser()
	{
		static_assert(sizeof(Field) == 3, "Invalid Field alignment");
		static_assert(sizeof(FileHeader) == 12, "Invalid FileHeader alignment");
	}

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	struct Lap
	{
		Lap() : trigger(0xff), event(0xff), eventType(0xff) {}

		quint8 trigger;
		quint8 event;
		quint8 eventType;
	};
	struct Event
	{
		Event() : data(0), id(0xff), type(0xff) {}

		quint32 data;
		quint8 id;
		quint8 type;
	};
	struct FileHeader
	{
		quint8 headerSize;
		quint8 protocolVersion;
		quint16 profileVersion;
		quint32 dataSize;
		quint32 magic;
	};
	struct Field
	{
		quint8 id;
		quint8 size;
		quint8 type;
	};
	struct MessageDefinition
	{
		MessageDefinition() : globalId(0), endian(0) {}

		QVector<Field> fields;
		QVector<Field> devFields;
		quint16 globalId;
		quint8 endian;
	};
	struct CTX {
		CTX(QFile *file, QVector<Waypoint> &waypoints)
		  : file(file), waypoints(waypoints), len(0), endian(0), timestamp(0),
		  ratio(NAN), laps(0), segment(false) {}

		QFile *file;
		QVector<Waypoint> &waypoints;
		TrackData track;
		quint32 len;
		quint8 endian;
		quint32 timestamp;
		MessageDefinition defs[16];
		qreal ratio;
		Trackpoint trackpoint;
		unsigned laps;
		bool segment;
	};

	bool readData(QFile *file, char *data, size_t size);
	template<class T> bool readValue(CTX &ctx, T &val);
	bool skipValue(CTX &ctx, quint8 size);
	bool readField(CTX &ctx, const Field *field, QVariant &val, bool &valid);

	bool parseHeader(CTX &ctx);
	bool parseRecord(CTX &ctx);
	bool parseDefinitionMessage(CTX &ctx, quint8 header);
	bool parseCompressedMessage(CTX &ctx, quint8 header);
	bool parseDataMessage(CTX &ctx, quint8 header);
	bool parseData(CTX &ctx, const MessageDefinition *def);

	QString _errorString;
};

#endif // FITPARSER_H
