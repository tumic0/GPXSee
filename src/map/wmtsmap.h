#ifndef WMTSMAP_H
#define WMTSMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"
#include "wmts.h"

class TileLoader;

class WMTSMap : public Map
{
	Q_OBJECT

public:
	WMTSMap(const QString &name, const WMTS::Setup &setup, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, bool block);

	void clearCache();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	bool loadWMTS();
	double sd2res(double scaleDenominator) const;
	QString tilesDir() const;
	void updateTransform();

	QString _name;
	WMTS::Setup _setup;
	TileLoader *_tileLoader;
	RectC _bounds;
	QList<WMTS::Zoom> _zooms;
	Projection _projection;
	Transform _transform;
	CoordinateSystem _cs;
	int _zoom;

	bool _valid;
	QString _errorString;
};

#endif // WMTSMAP_H
