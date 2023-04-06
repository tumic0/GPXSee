#ifndef MAPSFORGE_RASTERTILE_H
#define MAPSFORGE_RASTERTILE_H

#include <QPixmap>
#include "map/projection.h"
#include "map/transform.h"
#include "map/textpointitem.h"
#include "map/textpathitem.h"
#include "style.h"
#include "mapdata.h"

namespace Mapsforge {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform, int zoom,
	  const QRect &rect, qreal ratio, const QList<MapData::Path> &paths,
	  const QList<MapData::Point> &points) : _proj(proj), _transform(transform),
	  _zoom(zoom), _rect(rect), _ratio(ratio),
	  _pixmap(rect.width() * ratio, rect.height() * ratio), _paths(paths),
	  _points(points), _valid(false) {}

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}
	bool isValid() const {return _valid;}

	void render();

private:
	struct PainterPath {
		PainterPath() : path(0) {}

		QPainterPath pp;
		const MapData::Path *path;
	};

	struct PainterPoint {
		PainterPoint(const MapData::Point *p, const QByteArray *lbl,
		  const Style::Symbol *si, const Style::TextRender *ti)
		  : p(p), lbl(lbl), ti(ti), si(si)
		{
			Q_ASSERT(si || ti);
		}

		bool operator<(const PainterPoint &other) const
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

	class PathInstruction
	{
	public:
		PathInstruction() : _render(0), _path(0) {}
		PathInstruction(const Style::PathRender *render, PainterPath *path)
		  : _render(render), _path(path) {}

		bool operator<(const PathInstruction &other) const
		{
			if (_path->path->layer == other._path->path->layer)
				return _render->zOrder() < other._render->zOrder();
			else
				return (_path->path->layer < other._path->path->layer);
		}

		const Style::PathRender *render() const {return _render;}
		PainterPath *path() const {return _path;}

	private:
		const Style::PathRender *_render;
		PainterPath *_path;
	};

	struct Key {
		Key(int zoom, bool closed, const QVector<MapData::Tag> &tags)
		  : zoom(zoom), closed(closed), tags(tags) {}
		bool operator==(const Key &other) const
		{
			return zoom == other.zoom && closed == other.closed
			  && tags == other.tags;
		}

		int zoom;
		bool closed;
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
		  const QRect &tileRect, const QFont *font, const QColor *color,
		  const QColor *haloColor) : TextPathItem(line,
		  label ? new QString(*label) : 0, tileRect, font, color, haloColor) {}
		~PathItem() {delete _text;}
	};

	friend HASH_T qHash(const RasterTile::Key &key);

	QVector<PathInstruction> pathInstructions(QVector<PainterPath> &paths);
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	void processPointLabels(QList<TextItem*> &textItems);
	void processAreaLabels(QList<TextItem*> &textItems,
	  QVector<PainterPath> &paths);
	void processLineLabels(QList<TextItem*> &textItems,
	  QVector<PainterPath> &paths);
	QPainterPath painterPath(const Polygon &polygon, bool curve) const;
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);
	void drawPaths(QPainter *painter, QVector<PainterPath> &paths);

	Projection _proj;
	Transform _transform;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
	QList<MapData::Path> _paths;
	QList<MapData::Point> _points;

	bool _valid;
};

inline HASH_T qHash(const RasterTile::Key &key)
{
	return ::qHash(key.zoom) ^ ::qHash(key.tags);
}

}

#endif // MAPSFORGE_RASTERTILE_H
