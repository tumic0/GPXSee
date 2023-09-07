#ifndef ENCJOB_H
#define ENCJOB_H

#include <QtConcurrent>
#include "ENC/rastertile.h"

class ENCJob : public QObject
{
	Q_OBJECT

public:
	ENCJob(const QList<ENC::RasterTile> &tiles)
	  : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &ENCJob::handleFinished);
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
	void finished(ENCJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<ENC::RasterTile> _tiles;
};

#endif // ENCJOB_H
