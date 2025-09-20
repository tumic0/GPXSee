#ifndef MAPSFORGEMAP_H
#define MAPSFORGEMAP_H

#include <QtConcurrent>
#include "mapsforge/mapdata.h"
#include "mapsforge/rastertile.h"
#include "projection.h"
#include "transform.h"
#include "map.h"


class MapsforgeMapJob : public QObject
{
	Q_OBJECT

public:
	MapsforgeMapJob(const QList<Mapsforge::RasterTile> &tiles)
	  : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &MapsforgeMapJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &Mapsforge::RasterTile::render);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<Mapsforge::RasterTile> &tiles() const {return _tiles;}

signals:
	void finished(MapsforgeMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<Mapsforge::RasterTile> _tiles;
};

class MapsforgeMap : public Map
{
	Q_OBJECT

public:
	MapsforgeMap(const QString &fileName, QObject *parent = 0);

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _data.bounds();}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi, int layer);
	void unload();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	QStringList layers(const QString &lang, int &defaultLayer) const;
	bool hillShading() const;

	bool isValid() const {return _data.isValid();}
	QString errorString() const {return _data.errorString();}

	static Map *create(const QString &path, const Projection &proj, bool *isMap);

private slots:
	void jobFinished(MapsforgeMapJob *job);

private:
	QString key(int zoom, const QPoint &xy) const;
	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(MapsforgeMapJob *job);
	void removeJob(MapsforgeMapJob *job);
	void cancelJobs(bool wait);

	Mapsforge::MapData _data;
	Mapsforge::Style _style;
	int _zoom;

	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	qreal _tileRatio;

	QList<MapsforgeMapJob*> _jobs;
};

#endif // MAPSFORGEMAP_H
