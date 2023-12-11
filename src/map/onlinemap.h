#ifndef ONLINEMAP_H
#define ONLINEMAP_H

#include <QImageReader>
#include <QPixmap>
#include <QtConcurrent>
#include "common/range.h"
#include "common/rectc.h"
#include "map.h"
#include "tileloader.h"

class OnlineMapTile
{
public:
	OnlineMapTile(const QPoint &xy, const QString &file, int zoom, int overzoom,
	  int scaledSize, const QString &key) : _zoom(zoom), _overzoom(overzoom),
	  _scaledSize(scaledSize), _xy(xy), _file(file), _key(key) {}

	void load()
	{
		QByteArray format(_overzoom
		  ? QByteArray::number(_zoom) + ';' + QByteArray::number(_overzoom)
		  : QByteArray::number(_zoom));
		QImageReader reader(_file, format);
		if (_scaledSize)
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
		_pixmap = QPixmap::fromImage(reader.read());
	}

	const QPoint &xy() const {return _xy;}
	const QPixmap &pixmap() const {return _pixmap;}
	const QString &key() const {return _key;}

private:
	int _zoom;
	int _overzoom;
	int _scaledSize;
	QPoint _xy;
	QString _file;
	QString _key;
	QPixmap _pixmap;
};

class OnlineMapJob : public QObject
{
	Q_OBJECT

public:
	OnlineMapJob(const QList<OnlineMapTile> &tiles) : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &OnlineMapJob::handleFinished);
		_future = QtConcurrent::map(_tiles, &OnlineMapTile::load);
		_watcher.setFuture(_future);
	}
	void cancel(bool wait)
	{
		_future.cancel();
		if (wait)
			_future.waitForFinished();
	}
	const QList<OnlineMapTile> &tiles() const {return _tiles;}

signals:
	void finished(OnlineMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<OnlineMapTile> _tiles;
};

class OnlineMap : public Map
{
	Q_OBJECT

public:
	OnlineMap(const QString &fileName, const QString &name, const QString &url,
	  const Range &zooms, const RectC &bounds, qreal tileRatio,
	  const QList<HTTPHeader> &headers, int tileSize, bool scalable,
	  bool invertY, bool quadTiles, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();
	void clearCache();

private slots:
	void jobFinished(OnlineMapJob *job);

private:
	int limitZoom(int zoom) const;
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QPoint tileCoordinates(int x, int y, int zoom);
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);
	bool isRunning(const QString &key) const;
	void runJob(OnlineMapJob *job);
	void removeJob(OnlineMapJob *job);
	void cancelJobs(bool wait);

	TileLoader *_tileLoader;
	QString _name;
	Range _zooms;
	RectC _bounds;
	int _zoom;
	int _tileSize;
	int _baseZoom;
	qreal _mapRatio, _tileRatio;
	bool _scalable;
	int _scaledSize;
	bool _invertY;

	QList<OnlineMapJob*> _jobs;
};

#endif // ONLINEMAP_H
