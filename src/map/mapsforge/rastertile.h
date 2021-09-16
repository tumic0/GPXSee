#ifndef MAPSFORGE_RASTERTILE_H
#define MAPSFORGE_RASTERTILE_H

#include <QPixmap>
#include "map/projection.h"
#include "map/transform.h"
#include "style.h"
#include "mapdata.h"

class MapsforgeMap;
class TextItem;

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
	class PathInstruction
	{
	public:
		PathInstruction() : _render(0), _path(0) {}
		PathInstruction(const Style::PathRender *render, MapData::Path *path)
		  : _render(render), _path(path) {}

		bool operator<(const PathInstruction &other) const
		{
			if (_path->layer == other._path->layer)
				return _render->zOrder() < other._render->zOrder();
			else
				return (_path->layer < other._path->layer);
		}

		const Style::PathRender *render() const {return _render;}
		MapData::Path *path() {return _path;}

	private:
		const Style::PathRender *_render;
		MapData::Path *_path;
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

	friend HASH_T qHash(const RasterTile::Key &key);
	friend HASH_T qHash(const RasterTile::PathInstruction &pi);

	QVector<PathInstruction> pathInstructions();
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	void processPointLabels(QList<TextItem*> &textItems);
	void processAreaLabels(QList<TextItem*> &textItems);
	void processLineLabels(QList<TextItem*> &textItems);
	QPainterPath painterPath(const Polygon &polygon) const;
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems);
	void drawPaths(QPainter *painter);

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
