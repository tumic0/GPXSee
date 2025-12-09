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
class Projection;

class MapSource
{
public:
	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private:
	enum Type {
		OSM,
		WMTS,
		WMS,
		TMS,
		QuadTiles
	};

	struct Tile {
		Tile() : size(256), ratio(1.0), mvt(false) {}

		int size;
		qreal ratio;
		bool mvt;
		QStringList vectorLayers;
	};

	struct Config {
		Type type;
		QString name;
		QStringList urls;
		QList<Tile> tiles;
		Range zooms;
		RectC bounds;
		QString layer;
		QString style;
		QString set;
		QString format;
		QString crs;
		CoordinateSystem coordinateSystem;
		bool rest;
		QList<KV<QString, QString> > dimensions;
		QList<HTTPHeader> headers;

		Config();
	};

	static RectC bounds(QXmlStreamReader &reader);
	static Range zooms(QXmlStreamReader &reader);
	static void map(QXmlStreamReader &reader, Config &config);
	static void tile(QXmlStreamReader &reader, Config &config, int layer);
};

#endif // MAPSOURCE_H
