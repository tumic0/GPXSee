#ifndef RASTERTILE_H
#define RASTERTILE_H

#include <QImage>
#include "mapdata.h"

class QPainter;
class TextItem;
class Style;

class RasterTile
{
public:
	RasterTile(const Style *style, int zoom, const QRect &rect,
	  const QString &key, const QList<MapData::Poly> &polygons,
	  const QList<MapData::Poly> &lines, QList<MapData::Point> &points)
	  : _style(style), _zoom(zoom), _xy(rect.topLeft()),
	  _key(key), _img(rect.size(), QImage::Format_ARGB32_Premultiplied),
	  _polygons(polygons), _lines(lines), _points(points) {}

	const QString &key() const {return _key;}
	const QPoint &xy() const {return _xy;}
	const QImage &img() const {return _img;}

	void render();

private:
	void drawPolygons(QPainter *painter);
	void drawLines(QPainter *painter);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	void processPolygons(QList<TextItem *> &textItems);
	void processLines(QList<TextItem*> &textItems);
	void processPoints(QList<TextItem*> &textItems);
	void processShields(const QRect &tileRect, QList<TextItem*> &textItems);
	void processStreetNames(const QRect &tileRect, QList<TextItem*> &textItems);

	const Style *_style;
	int _zoom;
	QPoint _xy;
	QString _key;
	QImage _img;
	QList<MapData::Poly> _polygons;
	QList<MapData::Poly> _lines;
	QList<MapData::Point> _points;
};

#endif // RASTERTILE_H
