#ifndef FITPARSER_H
#define FITPARSER_H

#include "parser.h"

class FITParser : public Parser
{
public:
	FITParser();

	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	struct FileHeader {
		quint8 headerSize;
		quint8 protocolVersion;
		quint16 profileVersion;
		quint32 dataSize;
		quint32 magic;
	};

	struct Field {
		quint8 id;
		quint8 size;
		quint8 type;
	};

	struct MessageDefinition {
		quint8 endian;
		quint16 globalId;
		quint8 numFields;
		Field *fields;
		quint8 numDevFields;
		Field *devFields;
	};


	void warning(const char *text) const;
	void clearDefinitions();

	bool readData(char *data, size_t size);
	template<class T> bool readValue(T &val);
	bool skipValue(size_t size);

	bool parseHeader();
	bool parseRecord(TrackData &track);
	bool parseDefinitionMessage(quint8 header);
	bool parseCompressedMessage(TrackData &track, quint8 header);
	bool parseDataMessage(TrackData &track, quint8 header);
	bool parseData(TrackData &track, MessageDefinition *def, quint8 offset);
	bool readField(Field *f, quint32 &val);

	QIODevice *_device;
	QString _errorString;

	quint32 _len;
	quint8 _endian;
	quint32 _timestamp;
	MessageDefinition _defs[16];
};

#endif // FITPARSER_H
