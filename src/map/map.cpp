#include <cmath>
#include <QLineF>
#include "map.h"


#define SAMPLES 100

void Map::growLeft(const QPointF &p, RectC &rect)
{
	Coordinates c(xy2ll(p));

	if (c.lon() < rect.left())
		rect.setLeft(c.lon());
}

void Map::growRight(const QPointF &p, RectC &rect)
{
	Coordinates c(xy2ll(p));

	if (c.lon() > rect.right())
		rect.setRight(c.lon());
}

void Map::growTop(const QPointF &p, RectC &rect)
{
	Coordinates c(xy2ll(p));

	if (c.lat() > rect.top())
		rect.setTop(c.lat());
}

void Map::growBottom(const QPointF &p, RectC &rect)
{
	Coordinates c(xy2ll(p));

	if (c.lat() < rect.bottom())
		rect.setBottom(c.lat());
}

RectC Map::llBounds()
{
	QRectF b(bounds());
	double dx = b.width() / SAMPLES;
	double dy = b.height() / SAMPLES;

	Coordinates tl(xy2ll(b.topLeft()));
	Coordinates br(xy2ll(b.bottomRight()));
	RectC rect(tl, br);

	for (int i = 0; i <= SAMPLES; i++) {
		double x = b.left() + i * dx;
		growBottom(QPointF(x, b.bottom()), rect);
		growTop(QPointF(x, b.top()), rect);
	}

	for (int i = 0; i <= SAMPLES; i++) {
		double y = b.top() + i * dy;
		growLeft(QPointF(b.left(), y), rect);
		growRight(QPointF(b.right(), y), rect);
	}

	return rect;
}

qreal Map::resolution(const QRectF &rect)
{
	qreal cy = rect.center().y();
	QPointF cl(rect.left(), cy);
	QPointF cr(rect.right(), cy);

	qreal ds = xy2ll(cl).distanceTo(xy2ll(cr));
	qreal ps = QLineF(cl, cr).length();

	return ds/ps;
}
