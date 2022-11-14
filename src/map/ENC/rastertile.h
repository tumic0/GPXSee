#ifndef ENC_RASTERTILE_H
#define ENC_RASTERTILE_H

#include <QPixmap>
#include "map/projection.h"
#include "map/transform.h"
#include "mapdata.h"

class TextItem;

namespace ENC {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform, int zoom,
	  const QRect &rect, qreal ratio, const QList<MapData::Line*> &lines,
	  const QList<MapData::Poly*> &polygons, const QList<MapData::Point*> &points)
	  : _proj(proj), _transform(transform), _zoom(zoom), _rect(rect),
	  _ratio(ratio), _pixmap(rect.width() * ratio, rect.height() * ratio),
	  _lines(lines), _polygons(polygons), _points(points), _valid(false) {}

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}
	bool isValid() const {return _valid;}

	void render();

private:
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	QPainterPath painterPath(const Polygon &polygon) const;
	QPolygonF polyline(const QVector<Coordinates> &path) const;
	QPolygonF arrow(const Coordinates &c, qreal angle) const;
	void processPoints(QList<TextItem*> &textItems);
	void processLines(QList<TextItem*> &textItems);
	void drawBitmapPath(QPainter *painter, const QImage &img,
	  const Polygon &polygon);
	void drawArrows(QPainter *painter);
	void drawPolygons(QPainter *painter);
	void drawLines(QPainter *painter);
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);

	Projection _proj;
	Transform _transform;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
	QList<MapData::Line*> _lines;
	QList<MapData::Poly*> _polygons;
	QList<MapData::Point*> _points;
	bool _valid;
};

}

#endif // ENC_RASTERTILE_H
