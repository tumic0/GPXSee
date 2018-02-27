#ifndef MAPSOURCE_H
#define MAPSOURCE_H

#include <QList>
#include "common/range.h"
#include "common/rectc.h"

class Map;
class QXmlStreamReader;

class MapSource
{
public:
	Map *loadFile(const QString &path);
	const QString &errorString() const {return _errorString;}

private:
	enum Type {
		TMS,
		WMTS
	};

	struct TMSConfig {
		Range zooms;
		RectC bounds;

		TMSConfig();
	};

	struct WMTSConfig {
		QString layer;
		QString style;
		QString set;
		QString format;
		bool rest;
		bool yx;

		WMTSConfig() : format("image/png"), rest(false), yx(false) {}
	};

	struct Config {
		QString name;
		QString url;
		Type type;
		WMTSConfig wmts;
		TMSConfig tms;

		Config() : type(TMS) {}
	};

	RectC bounds(QXmlStreamReader &reader);
	Range zooms(QXmlStreamReader &reader);
	void map(QXmlStreamReader &reader, Config &config);

	QString _errorString;
};

#endif // MAPSOURCE_H
