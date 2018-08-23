#ifndef EMPTYMAP_H
#define EMPTYMAP_H

#include "map.h"

class EmptyMap : public Map
{
	Q_OBJECT

public:
	EmptyMap(QObject *parent = 0);

	QString name() const {return QString();}

	QRectF bounds();
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

private:
	int _zoom;
};

#endif // EMPTYMAP_H
