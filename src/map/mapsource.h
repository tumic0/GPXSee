#ifndef MAPSOURCE_H
#define MAPSOURCE_H

#include <QList>
#include "common/range.h"
#include "common/rectc.h"
#include "common/kv.h"
#include "downloader.h"
#include "coordinatesystem.h"

class Map;
class QXmlStreamReader;

class MapSource
{
public:
	static Map *loadMap(const QString &path, QString &errorString);

private:
	enum Type {
		OSM,
		WMTS,
		WMS,
		TMS
	};

	struct Config {
		Type type;
		QString name;
		QString url;
		Range zooms;
		RectC bounds;
		QString layer;
		QString style;
		QString set;
		QString format;
		QString crs;
		CoordinateSystem coordinateSystem;
		bool rest;
		QList<KV> dimensions;
		Authorization authorization;
		qreal tileRatio;

		Config();
	};

	static RectC bounds(QXmlStreamReader &reader);
	static Range zooms(QXmlStreamReader &reader);
	static void map(QXmlStreamReader &reader, Config &config);
};

#endif // MAPSOURCE_H
