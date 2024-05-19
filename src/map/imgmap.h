#ifndef IMGMAP_H
#define IMGMAP_H

#include <QtConcurrent>
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "IMG/mapdata.h"
#include "IMG/rastertile.h"


class IMGMapJob : public QObject
{
	Q_OBJECT

public:
	IMGMapJob(const QList<IMG::RasterTile> &tiles)
	  : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &IMGMapJob::handleFinished);
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
	void finished(IMGMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<IMG::RasterTile> _tiles;
};

class IMGMap : public Map
{
	Q_OBJECT

public:
	IMGMap(const QString &fileName, bool GMAP, QObject *parent = 0);
	~IMGMap() {qDeleteAll(_data);}

	QString name() const {return _data.first()->name();}

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _data.first()->bounds();}

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
	  bool hidpi);
	void unload();

	double elevation(const Coordinates &c);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map* createIMG(const QString &path, const Projection &proj,
	  bool *isDir);
	static Map* createGMAP(const QString &path, const Projection &proj,
	  bool *isDir);

private slots:
	void jobFinished(IMGMapJob *job);

private:
	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(const QString &key) const;
	void runJob(IMGMapJob *job);
	void removeJob(IMGMapJob *job);
	void cancelJobs(bool wait);

	QList<IMG::MapData *> _data;
	int _zoom;
	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	RectC _dataBounds;
	qreal _tileRatio;

	QList<IMGMapJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // IMGMAP_H
