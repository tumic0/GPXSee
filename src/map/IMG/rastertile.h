#ifndef IMG_RASTERTILE_H
#define IMG_RASTERTILE_H

#include <QPixmap>
#include "mapdata.h"
#include "map/projection.h"
#include "map/transform.h"
#include "map/matrix.h"
#include "style.h"

class QPainter;
class TextItem;

namespace IMG {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform, MapData *data,
	  int zoom, const QRect &rect, qreal ratio, const QString &key,
	  bool hillShading, bool rasters, bool vectors)
		: _proj(proj), _transform(transform), _data(data), _zoom(zoom),
		_rect(rect), _ratio(ratio), _key(key), _hillShading(hillShading),
		_rasters(rasters), _vectors(vectors), _file(0) {}
	~RasterTile() {delete _file;}

	const QString &key() const {return _key;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	struct Sector
	{
		Sector(Light::Color color, quint32 start, quint32 end)
		  : color(color), start(start), end(end) {}

		bool operator==(const Sector &other) const
		{
			return (color == other.color && start == other.start
			  && end == other.end);
		}
		bool operator<(const Sector &other) const
		{
			if (color == other.color) {
				if (start == other.start)
					return end < other.end;
				else
					return start < other.start;
			} else
				return color < other.color;
		}

		Light::Color color;
		quint32 start;
		quint32 end;
	};

	void fetchData(QList<MapData::Poly> &polygons, QList<MapData::Poly> &lines,
	  QList<MapData::Point> &points);
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p) const
	  {return _proj.xy2ll(_transform.img2proj(p));}
	void ll2xy(QList<MapData::Poly> &polys) const;
	void ll2xy(QList<MapData::Point> &points) const;

	void drawPolygons(QPainter *painter,
	  const QList<MapData::Poly> &polygons) const;
	void drawLines(QPainter *painter, const QList<MapData::Poly> &lines) const;
	void drawTextItems(QPainter *painter,
	  const QList<TextItem*> &textItems) const;
	void drawHillShading(QPainter *painter) const;
	void drawSectorLights(QPainter *painter,
	  const QList<const MapData::Point*> &lights) const;

	void processPolygons(const QList<MapData::Poly> &polygons,
	  QList<TextItem*> &textItems);
	void processLines(QList<MapData::Poly> &lines, QList<TextItem*> &textItems,
	  const QImage (&arrows)[2]);
	void processPoints(QList<MapData::Point> &points,
	  QList<TextItem*> &textItems, QList<TextItem*> &lights,
	  QList<const MapData::Point*> &sectorLights);
	void processShields(const QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems);
	void processStreetNames(const QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems, const QImage (&arrows)[2]);

	const QFont *poiFont(Style::FontSize size = Style::Normal,
	  int zoom = -1, bool extended = false) const;

	MatrixD elevation(int extend) const;

	Projection _proj;
	Transform _transform;
	MapData *_data;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QString _key;
	QPixmap _pixmap;
	bool _hillShading;
	bool _rasters, _vectors;
	QFile *_file;
};

}

#endif // IMG_RASTERTILE_H
