#ifndef ENC_RASTERTILE_H
#define ENC_RASTERTILE_H

#include <QPixmap>
#include "common/range.h"
#include "map/projection.h"
#include "map/transform.h"
#include "data.h"
#include "style.h"
#include "atlasdata.h"

class TextItem;

namespace ENC {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform,
	  const Style *style, Data *data, int zoom,
	  const Range &zoomRange, const QRect &rect, qreal ratio) :
		_proj(proj), _transform(transform), _style(style),
		_zoom(zoom), _zoomRange(zoomRange), _rect(rect), _ratio(ratio)
	{
		_data.append(data);
	}
	RasterTile(const Projection &proj, const Transform &transform,
	  const Style *style, const QList<Data*> &data, int zoom,
	  const Range &zoomRange, const QRect &rect, qreal ratio) :
		_proj(proj), _transform(transform), _style(style), _data(data),
		_zoom(zoom), _zoomRange(zoomRange), _rect(rect), _ratio(ratio) {}

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	struct SectorLight
	{
		SectorLight(Style::Color color, uint visibility, double range,
		  double start, double end) : color(color), visibility(visibility),
		  range(range), start(start), end(end) {}

		Style::Color color;
		uint visibility;
		double range;
		double start;
		double end;
	};

	struct Level {
		QList<Data::Line> lines;
		QList<Data::Poly> polygons;
		QList<Data::Point> points;
		bool overZoom;

		bool isNull() const
		  {return lines.isEmpty() && polygons.isEmpty() && points.isEmpty();}
	};

	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	QPainterPath painterPath(const Polygon &polygon) const;
	QPolygonF polyline(const QVector<Coordinates> &path) const;
	QVector<QPolygonF> polylineM(const QVector<Coordinates> &path) const;
	QPolygonF tsslptArrow(const QPointF &p, qreal angle) const;
	QPointF centroid(const QVector<Coordinates> &polygon) const;
	void processPoints(const QList<Data::Point> &points,
	  QList<TextItem*> &textItems, QList<TextItem *> &lights,
	  QMap<Coordinates, SectorLight> &sectorLights, bool overZoom) const;
	void processLines(const QList<Data::Line> &lines,
	  QList<TextItem*> &textItems) const;
	void drawArrows(QPainter *painter, const QList<Data::Point> &points) const;
	void drawPolygons(QPainter *painter, const QList<Data::Poly> &polygons) const;
	void drawLines(QPainter *painter, const QList<Data::Line> &lines) const;
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems) const;
	void drawSectorLights(QPainter *painter,
	  const QMap<Coordinates, SectorLight> &lights) const;
	bool showLabel(const QImage *img, int type) const;
	void drawLevels(QPainter *painter, const QList<Level> &levels);
	QList<Level> fetchLevels();
	QPainterPath shape(const QList<Data::Poly> &polygons) const;

	Projection _proj;
	Transform _transform;
	const Style *_style;
	QList<Data *> _data;
	int _zoom;
	Range _zoomRange;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
};

}

#endif // ENC_RASTERTILE_H
