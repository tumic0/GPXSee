#ifndef PMTILEJOB_H
#define PMTILEJOB_H

#include <QtConcurrent>
#include "pmtile.h"

class PMTileJob : public QObject
{
	Q_OBJECT

public:
	PMTileJob(const QList<PMTile> &tiles) : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &PMTileJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &PMTile::load);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<PMTile> &tiles() const {return _tiles;}

signals:
	void finished(PMTileJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<PMTile> _tiles;
};

#endif // PMTILEJOB_H
