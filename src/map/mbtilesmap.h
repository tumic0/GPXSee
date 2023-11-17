#ifndef MBTILESMAP_H
#define MBTILESMAP_H

#include <QSqlDatabase>
#include <QVector>
#include <QImageReader>
#include <QBuffer>
#include <QPixmap>
#include <QtConcurrent>
#include "map.h"

class MBTile
{
public:
	MBTile(int zoom, int scaledSize, const QPoint &xy, const QByteArray &data,
	  const QString &key) : _zoom(zoom), _scaledSize(scaledSize), _xy(xy),
	  _data(data), _key(key) {}

	const QPoint &xy() const {return _xy;}
	const QString &key() const {return _key;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {
		QByteArray z(QString::number(_zoom).toLatin1());

		QBuffer buffer(&_data);
		QImageReader reader(&buffer, z);
		if (_scaledSize)
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
		_pixmap = QPixmap::fromImage(reader.read());
	}

private:
	int _zoom;
	int _scaledSize;
	QPoint _xy;
	QByteArray _data;
	QString _key;
	QPixmap _pixmap;
};

class MBTilesMapJob : public QObject
{
	Q_OBJECT

public:
	MBTilesMapJob(const QList<MBTile> &tiles) : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &MBTilesMapJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &MBTile::load);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<MBTile> &tiles() const {return _tiles;}

signals:
	void finished(MBTilesMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<MBTile> _tiles;
};

class MBTilesMap : public Map
{
	Q_OBJECT

public:
	MBTilesMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zi;}
	void setZoom(int zoom) {_zi = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(MBTilesMapJob *job);

private:
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QByteArray tileData(int zoom, const QPoint &tile) const;
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);
	bool isRunning(const QString &key) const;
	void runJob(MBTilesMapJob *job);
	void removeJob(MBTilesMapJob *job);
	void cancelJobs(bool wait);

	QSqlDatabase _db;

	QString _name;
	RectC _bounds;
	QVector<int> _zooms;
	int _zi;
	int _tileSize;
	qreal _mapRatio, _tileRatio;
	bool _scalable;
	int _scaledSize;

	QList<MBTilesMapJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // MBTILESMAP_H
