#ifndef MAPSFORGE_RASTERTILE_H
#define MAPSFORGE_RASTERTILE_H

#include <QPixmap>
#include "map/projection.h"
#include "map/transform.h"
#include "map/textpointitem.h"
#include "map/textpathitem.h"
#include "map/matrix.h"
#include "style.h"
#include "mapdata.h"


#define HILLSHADING_RENDER(ptr) \
	static_cast<const Style::HillShadingRender*>(ptr)
#define PATH_RENDER(ptr) \
	static_cast<const Style::PathRender*>(ptr)
#define POINT_RENDER(ptr) \
	static_cast<const Style::CircleRender*>(ptr)

namespace Mapsforge {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform,
	  const Style *style, MapData *data, int zoom, const QRect &rect,
	  qreal ratio, bool hillShading)
		: _proj(proj), _transform(transform), _style(style), _data(data),
		_zoom(zoom), _rect(rect), _ratio(ratio), _hillShading(hillShading) {}

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	struct PainterPath {
		PainterPath() : path(0) {}

		QPainterPath pp;
		const MapData::Path *path;
	};

	struct PointText {
		PointText(const MapData::Point *p, const QByteArray *lbl,
		  const Style::Symbol *si, const Style::TextRender *ti)
		  : p(p), lbl(lbl), ti(ti), si(si)
		{
			Q_ASSERT(si || ti);
		}

		bool operator<(const PointText &other) const
		{
			if (priority() == other.priority())
				return p->id < other.p->id;
			else
				return (priority() > other.priority());
		}
		int priority() const {return si ? si->priority() : ti->priority();}

		const MapData::Point *p;
		const QByteArray *lbl;
		const Style::TextRender *ti;
		const Style::Symbol *si;
	};

	struct PathText {
		PathText(const PainterPath *p, const QByteArray *lbl,
		  const Style::Symbol *si, const Style::TextRender *ti)
		  : p(p), lbl(lbl), ti(ti), si(si)
		{
			Q_ASSERT(si || ti);
		}

		bool operator<(const PathText &other) const
		{
			return (priority() > other.priority());
		}
		int priority() const {return si ? si->priority() : ti->priority();}

		const PainterPath *p;
		const QByteArray *lbl;
		const Style::TextRender *ti;
		const Style::Symbol *si;
	};

	class RenderInstruction
	{
	public:
		RenderInstruction() : _render(0), _path(0), _point(0) {}
		RenderInstruction(const Style::PathRender *render, PainterPath *path)
		  : _render(render), _path(path), _point(0) {}
		RenderInstruction(const Style::CircleRender *render,
		  const MapData::Point *point) : _render(render), _path(0),
		  _point(point) {}
		RenderInstruction(const Style::HillShadingRender *render)
		  : _render(render), _path(0), _point(0) {}

		bool operator<(const RenderInstruction &other) const
		{
			if (layer() == other.layer())
				return zOrder() < other.zOrder();
			else
				return (layer() < other.layer());
		}

		const Style::PathRender *pathRender() const
		  {return PATH_RENDER(_render);}
		const Style::CircleRender *circleRender() const
		  {return POINT_RENDER(_render);}
		const Style::HillShadingRender *hillShadingRender() const
		  {return HILLSHADING_RENDER(_render);}

		PainterPath *path() const {return _path;}
		const MapData::Point *point() const {return _point;}

	private:
		int layer() const
		{
			if (_path)
				return _path->path->layer;
			else if (_point)
				return _point->layer;
			else
				return HILLSHADING_RENDER(_render)->layer();
		}
		int zOrder() const
		{
			if (_path)
				return PATH_RENDER(_render)->zOrder();
			else if (_point)
				return POINT_RENDER(_render)->zOrder();
			else
				return HILLSHADING_RENDER(_render)->zOrder();
		}

		const Style::Render *_render;
		PainterPath *_path;
		const MapData::Point *_point;
	};

	struct PathKey {
		PathKey(int zoom, bool closed, const QVector<MapData::Tag> &tags)
		  : zoom(zoom), closed(closed), tags(tags) {}
		bool operator==(const PathKey &other) const
		{
			return zoom == other.zoom && closed == other.closed
			  && tags == other.tags;
		}

		int zoom;
		bool closed;
		const QVector<MapData::Tag> &tags;
	};

	struct PointKey {
		PointKey(int zoom, const QVector<MapData::Tag> &tags)
		  : zoom(zoom), tags(tags) {}
		bool operator==(const PointKey &other) const
		{
			return zoom == other.zoom && tags == other.tags;
		}

		int zoom;
		const QVector<MapData::Tag> &tags;
	};

	class PointItem : public TextPointItem
	{
	public:
		PointItem(const QPoint &point, const QByteArray *label,
		  const QFont *font, const QImage *img, const QColor *color,
		  const QColor *haloColor) : TextPointItem(point,
		  label ? new QString(*label) : 0, font, img, color, haloColor, 0) {}
		~PointItem() {delete _text;}
	};

	class PathItem : public TextPathItem
	{
	public:
		PathItem(const QPainterPath &line, const QByteArray *label,
		  const QImage *img, const QRect &tileRect, const QFont *font,
		  const QColor *color, const QColor *haloColor, bool rotate)
		  : TextPathItem(line, label ? new QString(*label) : 0, tileRect, font,
		  color, haloColor, img, rotate) {}
		~PathItem() {delete _text;}
	};

	friend HASH_T qHash(const RasterTile::PathKey &key);
	friend HASH_T qHash(const RasterTile::PointKey &key);

	void fetchData(QList<MapData::Path> &paths,
	  QList<MapData::Point> &points) const;
	void pathInstructions(const QList<MapData::Path> &paths,
	  QVector<PainterPath> &painterPaths,
	  QVector<RasterTile::RenderInstruction> &instructions) const;
	void circleInstructions(const QList<MapData::Point> &points,
	  QVector<RasterTile::RenderInstruction> &instructions) const;
	void hillShadingInstructions(
	  QVector<RasterTile::RenderInstruction> &instructions) const;
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p) const
	  {return _proj.xy2ll(_transform.img2proj(p));}
	void processPointLabels(const QList<MapData::Point> &points,
	  QList<TextItem*> &textItems) const;
	void processAreaLabels(const QVector<PainterPath> &paths,
	  QList<TextItem*> &textItems) const;
	void processLineLabels(const QVector<PainterPath> &paths,
	  QList<TextItem*> &textItems) const;
	QPainterPath painterPath(const Polygon &polygon, bool curve) const;
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);
	void drawPaths(QPainter *painter, const QList<MapData::Path> &paths,
	  const QList<MapData::Point> &points, QVector<PainterPath> &painterPaths);

	MatrixD elevation() const;

	Projection _proj;
	Transform _transform;
	const Style *_style;
	MapData *_data;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
	bool _hillShading;
};

inline HASH_T qHash(const RasterTile::PathKey &key)
{
	return ::qHash(key.zoom) ^ ::qHash(key.tags);
}

inline HASH_T qHash(const RasterTile::PointKey &key)
{
	return ::qHash(key.zoom) ^ ::qHash(key.tags);
}

}

#endif // MAPSFORGE_RASTERTILE_H
