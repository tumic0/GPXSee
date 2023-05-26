#ifndef ENC_RASTERTILE_H
#define ENC_RASTERTILE_H

#include <QPixmap>
#include "map/projection.h"
#include "map/transform.h"
#include "map/textpointitem.h"
#include "mapdata.h"

class TextItem;

namespace ENC {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform,
	  const MapData *data, int zoom, const QRect &rect, qreal ratio)
	  : _proj(proj), _transform(transform), _data(data), _zoom(zoom),
	  _rect(rect), _ratio(ratio),
	  _pixmap(rect.width() * ratio, rect.height() * ratio), _valid(false) {}

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}
	bool isValid() const {return _valid;}

	void render();

private:
	void fetchData(QList<MapData::Poly*> &polygons, QList<MapData::Line*> &lines,
	  QList<MapData::Point*> &points);
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	QPainterPath painterPath(const Polygon &polygon) const;
	QPolygonF polyline(const QVector<Coordinates> &path) const;
	QPolygonF tsslptArrow(const Coordinates &c, qreal angle) const;
	void processPoints(QList<MapData::Point *> &points,
	  QList<TextItem*> &textItems, QList<TextItem *> &lights);
	void processLines(const QList<MapData::Line *> &lines,
	  QList<TextItem*> &textItems);
	void processPolygons(const QList<MapData::Poly *> &polygons,
	  QList<TextItem*> &textItems);
	void drawBitmapPath(QPainter *painter, const QImage &img,
	  const Polygon &polygon);
	void drawArrows(QPainter *painter, const QList<MapData::Poly*> &polygons);
	void drawPolygons(QPainter *painter, const QList<MapData::Poly *> &polygons);
	void drawLines(QPainter *painter, const QList<MapData::Line *> &lines);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	Projection _proj;
	Transform _transform;
	const MapData *_data;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
	bool _valid;
};

}

#endif // ENC_RASTERTILE_H
