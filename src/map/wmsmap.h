#ifndef WMSMAP_H
#define WMSMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"
#include "wms.h"
#include "rectd.h"

class TileLoader;

class WMSMap : public Map
{
	Q_OBJECT

public:
	WMSMap(const QString &fileName, const QString &name, const WMS::Setup &setup,
	  int tileSize, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _wms->bbox();}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi, int layer);
	void clearCache();

	bool isReady() const {return _wms->isReady();}
	bool isValid() const {return _wms->isValid();}
	QString errorString() const {return _wms->errorString();}

private slots:
	void wmsReady();

private:
	QString tileUrl() const;
	double sd2res(double scaleDenominator) const;
	void computeZooms();
	void updateTransform();
	qreal tileSize() const;
	void init();
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	QString _name;
	WMS *_wms;
	TileLoader *_tileLoader;
	RectD _bounds;
	Transform _transform;
	QVector<double> _zooms;
	int _zoom;
	int _tileSize;
	qreal _mapRatio;
};

#endif // WMSMAP_H
