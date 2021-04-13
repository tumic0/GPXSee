#ifndef IMG_RASTERTILE_H
#define IMG_RASTERTILE_H

#include <QPixmap>
#include "mapdata.h"

class QPainter;
class IMGMap;
class TextItem;

namespace IMG {

class Style;

class RasterTile
{
public:
	RasterTile(IMGMap *map, const Style *style, int zoom, const QRect &rect,
	  const QString &key, const QList<MapData::Poly> &polygons,
	  const QList<MapData::Poly> &lines, QList<MapData::Point> &points)
	  : _map(map), _style(style), _zoom(zoom), _xy(rect.topLeft()),
	  _key(key), _pixmap(rect.size()), _polygons(polygons), _lines(lines),
	  _points(points) {}

	const QString &key() const {return _key;}
	const QPoint &xy() const {return _xy;}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	void ll2xy(QList<MapData::Poly> &polys);
	void ll2xy(QList<MapData::Point> &points);

	void drawPolygons(QPainter *painter);
	void drawLines(QPainter *painter);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	void processPolygons(QList<TextItem *> &textItems);
	void processLines(QList<TextItem*> &textItems);
	void processPoints(QList<TextItem*> &textItems);
	void processShields(const QRect &tileRect, QList<TextItem*> &textItems);
	void processStreetNames(const QRect &tileRect, QList<TextItem*> &textItems);

	IMGMap *_map;
	const Style *_style;
	int _zoom;
	QPoint _xy;
	QString _key;
	QPixmap _pixmap;
	QList<MapData::Poly> _polygons;
	QList<MapData::Poly> _lines;
	QList<MapData::Point> _points;
};

}

#endif // IMG_RASTERTILE_H
