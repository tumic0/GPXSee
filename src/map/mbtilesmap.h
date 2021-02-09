#ifndef MBTILESMAP_H
#define MBTILESMAP_H

#include <QSqlDatabase>
#include "common/range.h"
#include "map.h"

class MBTilesMap : public Map
{
public:
	MBTilesMap(const QString &fileName, QObject *parent = 0);

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

	void load();
	void unload();
	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	int limitZoom(int zoom) const;
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QByteArray tileData(int zoom, const QPoint &tile) const;
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	QSqlDatabase _db;

	QString _name;
	RectC _bounds;
	Range _zooms;
	int _zoom;
	int _tileSize;
	qreal _mapRatio, _tileRatio;
	bool _scalable;
	int _scaledSize;

	bool _valid;
	QString _errorString;
};

#endif // MBTILESMAP_H
