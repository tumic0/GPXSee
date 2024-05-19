#ifndef IMG_RASTERTILE_H
#define IMG_RASTERTILE_H

#include <QPixmap>
#include "mapdata.h"
#include "map/projection.h"
#include "map/transform.h"
#include "map/matrix.h"
#include "style.h"

class QPainter;
class IMGMap;
class TextItem;

namespace IMG {

class RasterTile
{
public:
	RasterTile(const Projection &proj, const Transform &transform, MapData *data,
	  int zoom, const QRect &rect, qreal ratio, const QString &key,
	  bool hillShading)
		: _proj(proj), _transform(transform), _data(data), _zoom(zoom),
		_rect(rect), _ratio(ratio), _key(key), _hillShading(hillShading) {}

	const QString &key() const {return _key;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	typedef RTree<const MapData::Elevation*, double, 2> DEMTRee;

	struct ElevationCTX {
		ElevationCTX(const DEMTRee &tree, const Coordinates &c, double &ele)
		  : tree(tree), c(c), ele(ele) {}

		const DEMTRee &tree;
		const Coordinates &c;
		double &ele;
	};
	struct EdgeCTX {
		EdgeCTX(const Coordinates &c, double &ele) : c(c), ele(ele) {}

		const Coordinates &c;
		double &ele;
	};

	void fetchData(QList<MapData::Poly> &polygons, QList<MapData::Poly> &lines,
	  QList<MapData::Point> &points);
	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.proj2img(_proj.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p) const
	  {return _proj.xy2ll(_transform.img2proj(p));}
	void ll2xy(QList<MapData::Poly> &polys) const;
	void ll2xy(QList<MapData::Point> &points) const;

	void drawPolygons(QPainter *painter, const QList<MapData::Poly> &polygons) const;
	void drawLines(QPainter *painter, const QList<MapData::Poly> &lines) const;
	void drawTextItems(QPainter *painter, const QList<TextItem*> &textItems) const;
	void drawHillShading(QPainter *painter) const;

	void processPolygons(const QList<MapData::Poly> &polygons,
	  QList<TextItem *> &textItems);
	void processLines(QList<MapData::Poly> &lines, QList<TextItem*> &textItems,
	  const QImage (&arrows)[2]);
	void processPoints(QList<MapData::Point> &points,
	  QList<TextItem*> &textItems);
	void processShields(const QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems);
	void processStreetNames(const QList<MapData::Poly> &lines,
	  QList<TextItem*> &textItems, const QImage (&arrows)[2]);

	const QFont *poiFont(Style::FontSize size = Style::Normal,
	  int zoom = -1, bool extended = false) const;

	MatrixD elevation() const;

	static double edge(const DEMTRee &tree, const Coordinates &c);
	static double elevation(const DEMTRee &tree, const MapData::Elevation *e,
	  const Coordinates &c);
	static void buildTree(const QList<MapData::Elevation> &tiles, DEMTRee &tree);
	static void searchTree(const DEMTRee &tree, const Coordinates &c,
	  double &ele);
	static bool elevationCb(const MapData::Elevation *e, void *context);
	static bool edgeCb(const MapData::Elevation *e, void *context);

	Projection _proj;
	Transform _transform;
	MapData *_data;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QString _key;
	QPixmap _pixmap;
	bool _hillShading;
};

}

#endif // IMG_RASTERTILE_H
