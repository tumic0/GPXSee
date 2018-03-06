#ifndef ONLINEMAP_H
#define ONLINEMAP_H

#include "common/range.h"
#include "common/rectc.h"
#include "map.h"
#include "tileloader.h"

class OnlineMap : public Map
{
	Q_OBJECT

public:
	OnlineMap(const QString &name, const QString &url, const Range &zooms,
	  const RectC &bounds, QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QRectF &rect) const;

	int zoom() const {return _zoom;}
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const OnlineMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const OnlineMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect);

	void setBlockingMode(bool block) {_block = block;}
	void clearCache() {_tileLoader.clearCache();}

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private slots:
	void emitLoaded();

private:
	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;
	int limitZoom(int zoom) const;

	TileLoader _tileLoader;
	QString _name;
	Range _zooms;
	RectC _bounds;
	int _zoom;
	bool _block;

	bool _valid;
	QString _errorString;
};

#endif // ONLINEMAP_H
