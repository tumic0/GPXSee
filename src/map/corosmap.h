#ifndef COROSMAP_H
#define COROSMAP_H

#include <QtConcurrent>
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "IMG/mapdata.h"
#include "IMG/rastertile.h"
#include "IMG/style.h"

class CorosMapJob : public QObject
{
	Q_OBJECT

public:
	CorosMapJob(const QList<IMG::RasterTile> &tiles)
	  : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &CorosMapJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &IMG::RasterTile::render);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<IMG::RasterTile> &tiles() const {return _tiles;}

signals:
	void finished(CorosMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<IMG::RasterTile> _tiles;
};

class CorosMap : public Map
{
	Q_OBJECT

public:
	CorosMap(const QString &fileName, QObject *parent = 0);
	~CorosMap();

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _dataBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &, const RectC &);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal devicelRatio,
	  bool hidpi, int layer);
	void unload();

	double elevation(const Coordinates &c);

	QStringList layers(const QString &lang, int &defaultLayer) const;
	bool hillShading() const {return true;}

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map* create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(CorosMapJob *job);

private:
	enum Layer {
		Landscape = 1,
		Topo = 2,
		All = 3
	};
	typedef RTree<IMG::IMGData*, double, 2> MapTree;

	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(const QString &key) const;
	void runJob(CorosMapJob *job);
	void removeJob(CorosMapJob *job);
	void cancelJobs(bool wait);

	void loadDir(const QString &path, MapTree &tree);

	MapTree _osm, _cm;
	Range _zooms;
	int _zoom;
	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	RectC _dataBounds;
	qreal _tileRatio;
	Layer _layer;
	IMG::Style *_style;
	IMG::MapData::PolyCache _polyCache;
	IMG::MapData::PointCache _pointCache;
	IMG::MapData::ElevationCache _demCache;
	QMutex _lock, _demLock;

	QList<CorosMapJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // COROSMAP_H
