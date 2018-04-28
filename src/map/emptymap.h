#ifndef EMPTYMAP_H
#define EMPTYMAP_H

#include "map.h"

class EmptyMap : public Map
{
	Q_OBJECT

public:
	EmptyMap(QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QRectF &rect) const;

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const EmptyMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const EmptyMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect, bool block);

private:
	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	QString _name;
	int _zoom;
};

#endif // EMPTYMAP_H
