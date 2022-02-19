#ifndef MBTILESMAP_H
#define MBTILESMAP_H

#include <QSqlDatabase>
#include <QVector>
#include "map.h"

class MBTilesMap : public Map
{
public:
	MBTilesMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zi;}
	void setZoom(int zoom) {_zi = zoom;}
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
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QByteArray tileData(int zoom, const QPoint &tile) const;
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	QSqlDatabase _db;

	QString _name;
	RectC _bounds;
	QVector<int> _zooms;
	int _zi;
	int _tileSize;
	qreal _mapRatio, _tileRatio;
	bool _scalable;
	int _scaledSize;

	bool _valid;
	QString _errorString;
};

#endif // MBTILESMAP_H
