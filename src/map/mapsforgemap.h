#ifndef MAPSFORGEMAP_H
#define MAPSFORGEMAP_H

#include <QtConcurrent>
#include <QPixmapCache>
#include "mapsforge/mapdata.h"
#include "mapsforge/rastertile.h"
#include "projection.h"
#include "transform.h"
#include "map.h"


class MapsforgeMapJob : public QObject
{
	Q_OBJECT

public:
	MapsforgeMapJob(const QList<Mapsforge::RasterTile> &tiles) : _tiles(tiles)
	{
		connect(&_watcher, &QFutureWatcher<void>::finished, this,
		  &MapsforgeMapJob::handleFinished);
	}

	void run()
	{
		_future = QtConcurrent::map(_tiles, &Mapsforge::RasterTile::render);
		_watcher.setFuture(_future);
	}

signals:
	void finished(const QList<Mapsforge::RasterTile> &);

private slots:
	void handleFinished()
	{
		for (int i = 0; i < _tiles.size(); i++) {
			Mapsforge::RasterTile &mt = _tiles[i];
			const QPixmap &pm = mt.pixmap();
			if (pm.isNull())
				continue;

			QPixmapCache::insert(mt.key(), pm);
		}

		emit finished(_tiles);

		deleteLater();
	}

private:
	QFutureWatcher<void> _watcher;
	QFuture<void> _future;
	QList<Mapsforge::RasterTile> _tiles;
};

class MapsforgeMap : public Map
{
	Q_OBJECT

public:
	MapsforgeMap(const QString &fileName, QObject *parent = 0);

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _data.bounds();}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	void load();
	void unload();
	void setOutputProjection(const Projection &projection);

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _data.isValid();}
	QString errorString() const {return _data.errorString();}

private slots:
	void jobFinished(const QList<Mapsforge::RasterTile> &tiles);

private:
	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(const QString &key) const;
	void addRunning(const QList<Mapsforge::RasterTile> &tiles);
	void removeRunning(const QList<Mapsforge::RasterTile> &tiles);

	Mapsforge::MapData _data;
	int _zoom;

	Projection _projection;
	Transform _transform;
	QRectF _bounds;

	QSet<QString> _running;
};

#endif // MAPSFORGEMAP_H
