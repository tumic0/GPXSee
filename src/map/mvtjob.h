#ifndef MVTJOB_H
#define MVTJOB_H

#include <QtConcurrentMap>
#include <QFutureWatcher>
#include "MVT/rastertile_mvt.h"

class MVTJob : public QObject
{
	Q_OBJECT

public:
	MVTJob(const QList<MVT::RasterTile> &tiles) : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &MVTJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &MVT::RasterTile::render);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<MVT::RasterTile> &tiles() const {return _tiles;}

signals:
	void finished(MVTJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<MVT::RasterTile> _tiles;
};

static constexpr int MAX_TILE_SIZE = 4096;
static constexpr int TILE_SIZE = 512;

#endif // MVTJOB_H
