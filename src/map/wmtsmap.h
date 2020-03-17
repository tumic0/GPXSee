#ifndef WMTSMAP_H
#define WMTSMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"
#include "rectd.h"
#include "wmts.h"

class TileLoader;

class WMTSMap : public Map
{
	Q_OBJECT

public:
	WMTSMap(const QString &name, const WMTS::Setup &setup, qreal tileRatio,
	  QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setDevicePixelRatio(qreal /*deviceRatio*/, qreal mapRatio)
	  {_mapRatio = mapRatio;}
	void clearCache();

	bool isReady() const {return _wmts->isReady();}
	bool isValid() const {return _wmts->isValid();}
	QString errorString() const {return _wmts->errorString();}

private slots:
	void wmtsReady();

private:
	double sd2res(double scaleDenominator) const;
	void updateTransform();
	QSizeF tileSize(const WMTS::Zoom &zoom) const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	void init();

	QString _name;
	WMTS *_wmts;
	TileLoader *_tileLoader;
	Transform _transform;
	RectD _bounds;
	int _zoom;
	qreal _mapRatio, _tileRatio;
};

#endif // WMTSMAP_H
