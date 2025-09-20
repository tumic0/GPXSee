#ifndef IMGJOB_H
#define IMGJOB_H

#include <QtConcurrent>
#include "IMG/rastertile.h"

class IMGJob : public QObject
{
	Q_OBJECT

public:
	IMGJob(const QList<IMG::RasterTile> &tiles)
	  : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &IMGJob::handleFinished);
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
	void finished(IMGJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<IMG::RasterTile> _tiles;
};

#endif // IMGJOB_H
