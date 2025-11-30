#ifndef MVTJOB_H
#define MVTJOB_H

#include <QtConcurrent>
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

#endif // MVTJOB_H
