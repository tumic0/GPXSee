#ifndef ENCMAP_H
#define ENCMAP_H

#include <QtConcurrent>
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "ENC/mapdata.h"
#include "ENC/rastertile.h"

class ENCMapJob : public QObject
{
	Q_OBJECT

public:
	ENCMapJob(const QList<ENC::RasterTile> &tiles)
	  : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &ENCMapJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &ENC::RasterTile::render);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<ENC::RasterTile> &tiles() const {return _tiles;}

signals:
	void finished(ENCMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<ENC::RasterTile> _tiles;
};

class ENCMap : public Map
{
	Q_OBJECT

public:
	ENCMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _data.name();}

	QRectF bounds() {return _bounds;}
	RectC llBounds(const Projection &) {return _llBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _data.isValid();}
	QString errorString() const {return _data.errorString();}

	static Map *create(const QString &path, bool *isMap);

private slots:
	void jobFinished(ENCMapJob *job);

private:
	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(ENCMapJob *job);
	void removeJob(ENCMapJob *job);
	void cancelJobs(bool wait);
	QString key(int zoom, const QPoint &xy) const;

	ENC::MapData _data;
	Projection _projection;
	Transform _transform;
	qreal _tileRatio;
	RectC _llBounds;
	QRectF _bounds;
	int _zoom;

	QList<ENCMapJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // ENCMAP_H
