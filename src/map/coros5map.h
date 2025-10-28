#ifndef COROS5MAP_H
#define COROS5MAP_H

#include "common/range.h"
#include "common/rtree.h"
#include "pmtiles.h"
#include "mvtjob.h"
#include "map.h"

class Coros5Map : public Map
{
	Q_OBJECT

public:
	Coros5Map(const QString &fileName, QObject *parent = 0);
	~Coros5Map();

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &, const RectC &);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal devicelRatio,
	  bool hidpi, int style, int layer);
	void unload();

	QStringList styles(int &defaultStyle) const;
	bool hillShading() const;

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(MVTJob *job);

private:
	struct MapTile {
		MapTile(const QString &path);

		bool isValid() const {return bounds.isValid() && zooms.isValid();}
		QStringList vectorLayers() const;

		QString path;
		RectC bounds;
		Range zooms;
		quint64 rootOffset, rootLength;
		quint64 tileOffset, leafOffset;
		quint64 metadataOffset, metadataLength;
		quint8 tc, ic, tt;
	};

	struct Zoom {
		Zoom() : z(-1), base(-1)  {}
		Zoom(int z, int base) : z(z), base(base) {}

		int z;
		int base;
	};

	struct CacheEntry {
		CacheEntry(const MapTile *map) : file(map->path)
		{
			if (file.open(QIODevice::ReadOnly))
				root = PMTiles::readDir(file, map->rootOffset, map->rootLength,
				  map->ic);
		}

		QFile file;
		QVector<PMTiles::Directory> root;
	};

	typedef RTree<MapTile*, double, 2> MapTree;

	QPointF tilePos(const QPointF &tl, const QPoint &tc, const QPoint &tile,
	  unsigned overzoom) const;
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);
	QByteArray tileData(const MapTile *map, quint64 id);

	QString key(int zoom, const QPoint &xy) const;
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(MVTJob *job);
	void removeJob(MVTJob *job);
	void cancelJobs(bool wait);

	void loadDir(const QString &path, Range &zooms);
	const MVT::Style *defaultStyle() const;

	static bool cb(MapTile *data, void *context);

	RectC _bounds;
	MapTree _maps;
	QVector<Zoom> _zooms, _zoomsBase;
	QCache<const MapTile*, CacheEntry> _cache;
	const MVT::Style *_style;
	int _zoom;
	int _tileSize;
	qreal _mapRatio, _tileRatio;
	bool _mvt;
	QStringList _layers;

	QList<MVTJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // COROS5MAP_H
