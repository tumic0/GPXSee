#ifndef WMTSMAP_H
#define WMTSMAP_H

#include <QTransform>
#include "projection.h"
#include "map.h"
#include "wmts.h"
#include "tileloader.h"


class WMTSMap : public Map
{
	Q_OBJECT

public:
	WMTSMap(const QString &name, const WMTS::Setup &setup, bool invertAxis,
	  QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return _zoom;}
	qreal zoomFit(const QSize &size, const RectC &br);
	qreal zoomFit(qreal resolution, const Coordinates &c);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const WMTSMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const WMTSMap &>(*this).xy2ll(p);}

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
	qreal sd2res(qreal scaleDenominator) const;
	void updateTransform();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	QString _name;
	WMTS::Setup _setup;
	TileLoader _tileLoader;
	RectC _bounds;
	QList<WMTS::Zoom> _zooms;
	bool _invertAxis;
	Projection _projection;
	QTransform _transform, _inverted;
	int _zoom;
	bool _block;

	bool _valid;
	QString _errorString;
};

#endif // WMTSMAP_H
