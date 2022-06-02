#ifndef IMG_RASTERTILE_H
#define IMG_RASTERTILE_H

#include <QPixmap>
#include "mapdata.h"
#include "map/projection.h"
#include "map/transform.h"

class QPainter;
class IMGMap;
class TextItem;

namespace IMG {

class Style;

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform,
	  const Style *style, int zoom, const QRect &rect, qreal ratio,
	  const QString &key, const QList<MapData::Poly> &polygons,
	  const QList<MapData::Poly> &lines, QList<MapData::Point> &points)
	  : _proj(proj), _transform(transform), _style(style), _zoom(zoom),
	  _rect(rect), _ratio(ratio), _key(key),
	  _pixmap(rect.width() * ratio, rect.height() * ratio), _polygons(polygons),
	  _lines(lines), _points(points), _valid(false) {}

	const QString &key() const {return _key;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}
	bool isValid() const {return _valid;}

	void render();

private:
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	void ll2xy(QList<MapData::Poly> &polys);
	void ll2xy(QList<MapData::Point> &points);

	void drawPolygons(QPainter *painter);
	void drawLines(QPainter *painter);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	void processPolygons(QList<TextItem *> &textItems);
	void processLines(QList<TextItem*> &textItems);
	void processPoints(QList<TextItem*> &textItems);
	void processShields(QList<TextItem*> &textItems);
	void processStreetNames(QList<TextItem*> &textItems);

	Projection _proj;
	Transform _transform;
	const Style *_style;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QString _key;
	QPixmap _pixmap;
	QList<MapData::Poly> _polygons;
	QList<MapData::Poly> _lines;
	QList<MapData::Point> _points;
	bool _valid;
};

}

#endif // IMG_RASTERTILE_H
