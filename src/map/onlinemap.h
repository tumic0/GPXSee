#ifndef ONLINEMAP_H
#define ONLINEMAP_H

#include "common/range.h"
#include "common/rectc.h"
#include "tileloader.h"
#include "mvtjob.h"
#include "map.h"

class QPixmap;

class OnlineMap : public Map
{
	Q_OBJECT

public:
	enum TileType {
		MVT,
		Raster
	};

	OnlineMap(const QString &fileName, const QString &name,
	  const QStringList &url, const QList<TileType> &tileType, int tileSize,
	  qreal tileRatio, const Range &zooms, const RectC &bounds,
	  const QList<HTTPHeader> &headers, bool invertY, bool quadTiles,
	  QStringList vectorLayers, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi, bool hillShading, int style, int layer);
	void unload();
	void clearCache();

	QStringList styles(int &defaultStyle) const;
	QStringList layers(const QString &lang, int &defaultLayer) const;
	bool hillShading() const;

private slots:
	void jobFinished(MVTJob *job);

private:
	int limitZoom(int zoom) const;
	qreal tileSize() const;
	QPoint tileCoordinates(int x, int y, int zoom) const;
	QPointF tilePos(const QPointF &tl, const QPoint &tc, const QPoint &tile,
	  unsigned overzoom) const;
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	QString key(int zoom, const QPoint &xy) const;
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(MVTJob *job);
	void removeJob(MVTJob *job);
	void cancelJobs(bool wait);

	const MVT::Style *defaultStyle() const;

	TileLoader *_tileLoader;
	QString _name;
	Range _zooms;
	RectC _bounds;
	int _tileSize;
	int _zoom, _baseZoom;
	qreal _mapRatio, _tileRatio;
	QList<TileType> _tileType;
	int _layers;
	bool _hillShading, _invertY;
	const MVT::Style *_style;
	QStringList _vectorLayers;

	qreal _factor;
	qreal _coordinatesRatio;

	QList<MVTJob*> _jobs;
};

#endif // ONLINEMAP_H
