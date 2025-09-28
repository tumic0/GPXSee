#ifndef PMTILESMAP_H
#define PMTILESMAP_H

#include <QFile>
#include <QtConcurrent>
#include <QImageReader>
#include "common/rectc.h"
#include "mvtstyle.h"
#include "map.h"

class PMTile
{
public:
	PMTile(int zoom, int overzoom, int scaledSize, int style, const QPoint &xy,
	  const QByteArray &data, quint8 tc, const QString &key)
	  : _zoom(zoom), _overzoom(overzoom), _scaledSize(scaledSize),
	  _style(style), _xy(xy), _data(data), _key(key), _tc(tc) {}

	const QPoint &xy() const {return _xy;}
	const QString &key() const {return _key;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {
		QByteArray data;

		if (_scaledSize) {
			QByteArray format(QByteArray::number(_zoom)
			  + ';' + QByteArray::number(_overzoom)
			  + ';' + QByteArray::number(_style));
			data = (_tc == 2) ? Util::gunzip(_data) : _data;
			QBuffer buffer(&data);
			QImageReader reader(&buffer, format);
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
			_pixmap = QPixmap::fromImageReader(&reader);
		} else {
			data = (_tc == 2) ? Util::gunzip(_data) : _data;
			QBuffer buffer(&data);
			QImageReader reader(&buffer);
			_pixmap = QPixmap::fromImageReader(&reader);
		}
	}

private:
	int _zoom;
	int _overzoom;
	int _scaledSize;
	int _style;
	QPoint _xy;
	QByteArray _data;
	QString _key;
	QPixmap _pixmap;
	quint8 _tc;
};

class PMTilesMapJob : public QObject
{
	Q_OBJECT

public:
	PMTilesMapJob(const QList<PMTile> &tiles) : _tiles(tiles) {}

	void run()
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &PMTilesMapJob::handleFinished);
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
	void finished(PMTilesMapJob *job);

private slots:
	void handleFinished() {emit finished(this);}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<PMTile> _tiles;
};


class PMTilesMap : public Map
{
public:
	Q_OBJECT

public:
	PMTilesMap(const QString &fileName, QObject *parent = 0);

	QString name() const;

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
	  bool hidpi, int style, int layer);
	void unload();

	QStringList styles(int &defaultStyle) const;

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(PMTilesMapJob *job);

private:
	struct Zoom {
		Zoom() : z(-1), base(-1)  {}
		Zoom(int z, int base) : z(z), base(base) {}

		int z;
		int base;
	};

	struct Directory {
		quint64 tileId;
		quint64 length;
		quint64 offset;
		quint32 runLength;
	};

	QByteArray readData(quint64 offset, quint64 size, quint8 compression);
	QVector<Directory> readDir(quint64 offset, quint64 size, quint8 compression);

	int defaultStyle(const QStringList &vectorLayers);

	QPointF tilePos(const QPointF &tl, const QPoint &tc, const QPoint &tile,
	  unsigned overzoom) const;
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QByteArray tileData(quint64 id);
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);
	bool isRunning(const QString &key) const;
	void runJob(PMTilesMapJob *job);
	void removeJob(PMTilesMapJob *job);
	void cancelJobs(bool wait);

	static const Directory *findDir(const QVector<Directory> &dir,
	  quint64 tileId);

	QFile _file;
	QString _name;
	RectC _bounds;
	QVector<Directory> _root;
	QCache<quint64, QVector<Directory> > _cache;
	quint64 _tileOffset, _leafOffset;
	quint8 _tc, _ic;
	QVector<Zoom> _zooms, _zoomsBase;
	QList<MVTStyle> _styles;
	int _zoom;
	int _tileSize;
	int _style;
	qreal _mapRatio, _tileRatio;
	bool _mvt;
	int _scaledSize;

	QList<PMTilesMapJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // PMTILESMAP_H
