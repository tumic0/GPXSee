#ifndef WMSMAP_H
#define WMSMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"
#include "wms.h"
#include "tileloader.h"


class WMSMap : public Map
{
	Q_OBJECT

public:
	WMSMap(const QString &name, const WMS::Setup &setup, QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QRectF &rect) const;

	int zoom() const {return _zoom;}
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const WMSMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const WMSMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect);

	void setBlockingMode(bool block) {_block = block;}
	void clearCache();

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private slots:
	void emitLoaded();

private:
	QString tileUrl(const QString &version) const;
	qreal sd2res(qreal scaleDenominator) const;
	QString tilesDir() const;
	void computeZooms(const RangeF &scaleDenominator);
	void updateTransform();
	bool loadWMS();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	QString _name;

	WMS::Setup _setup;
	TileLoader _tileLoader;
	Projection _projection;
	Transform _transform;
	CoordinateSystem _cs;
	QVector<qreal> _zooms;
	QRectF _boundingBox;
	int _zoom;
	bool _block;

	bool _valid;
	QString _errorString;
};

#endif // WMSMAP_H
