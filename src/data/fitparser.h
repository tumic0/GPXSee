#ifndef FITPARSER_H
#define FITPARSER_H

#include "parser.h"

class QFile;

class FITParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	struct Field;
	class MessageDefinition;
	class CTX;

	bool readData(QFile *file, char *data, size_t size);
	template<class T> bool readValue(CTX &ctx, T &val);
	bool skipValue(CTX &ctx, quint8 size);
	bool readField(CTX &ctx, Field *field, quint32 &val);

	bool parseHeader(CTX &ctx);
	bool parseRecord(CTX &ctx);
	bool parseDefinitionMessage(CTX &ctx, quint8 header);
	bool parseCompressedMessage(CTX &ctx, quint8 header);
	bool parseDataMessage(CTX &ctx, quint8 header);
	bool parseData(CTX &ctx, const MessageDefinition *def);

	QString _errorString;
};

#endif // FITPARSER_H
