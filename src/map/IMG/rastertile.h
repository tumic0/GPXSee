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
	RasterTile(const Projection &proj, const Transform &transform, MapData *data,
	  int zoom, const QRect &rect, qreal ratio, const QString &key)
		: _proj(proj), _transform(transform), _data(data), _zoom(zoom),
		_rect(rect), _ratio(ratio), _key(key),
		_pixmap(rect.width() * ratio, rect.height() * ratio), _valid(false) {}

	const QString &key() const {return _key;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}
	bool isValid() const {return _valid;}

	void render();

private:
	void fetchData(QList<MapData::Poly> &polygons, QList<MapData::Poly> &lines,
	  QList<MapData::Point> &points);
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	void ll2xy(QList<MapData::Poly> &polys);
	void ll2xy(QList<MapData::Point> &points);

	void drawPolygons(QPainter *painter, const QList<MapData::Poly> &polygons);
	void drawLines(QPainter *painter, const QList<MapData::Poly> &lines);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	void processPolygons(const QList<MapData::Poly> &polygons,
	  QList<TextItem *> &textItems);
	void processLines(QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems, const QImage &arrow, const QImage &waterArrow);
	void processPoints(QList<MapData::Point> &points,
	  QList<TextItem*> &textItems);
	void processShields(const QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems);
	void processStreetNames(const QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems, const QImage &arrow, const QImage &waterArrow);

	Projection _proj;
	Transform _transform;
	MapData *_data;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QString _key;
	QPixmap _pixmap;
	bool _valid;
};

}

#endif // IMG_RASTERTILE_H
