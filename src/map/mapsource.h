#ifndef MAPSOURCE_H
#define MAPSOURCE_H

#include <QList>
#include "common/range.h"
#include "common/rectc.h"
#include "downloader.h"
#include "coordinatesystem.h"

class Map;
class QXmlStreamReader;

class MapSource
{
public:
	Map *loadFile(const QString &path);
	const QString &errorString() const {return _errorString;}

private:
	enum Type {
		OSM,
		WMTS,
		WMS
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
		QList<QPair<QString, QString> > dimensions;
		Authorization authorization;

		Config();
	};

	RectC bounds(QXmlStreamReader &reader);
	Range zooms(QXmlStreamReader &reader);
	void map(QXmlStreamReader &reader, Config &config);

	QString _errorString;
};

#endif // MAPSOURCE_H
