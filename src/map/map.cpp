#include <cmath>
#include <QLineF>
#include "map.h"


#define SAMPLES 100

static void growLeft(const Coordinates &c, RectC &rect)
{
	if (c.lon() < rect.left())
		rect.setLeft(c.lon());
}

static void growRight(const Coordinates &c, RectC &rect)
{
	if (c.lon() > rect.right())
		rect.setRight(c.lon());
}

static void growTop(const Coordinates &c, RectC &rect)
{
	if (c.lat() > rect.top())
		rect.setTop(c.lat());
}

static void growBottom(const Coordinates &c, RectC &rect)
{
	if (c.lat() < rect.bottom())
		rect.setBottom(c.lat());
}

RectC Map::llBounds(const Projection &proj)
{
	Q_UNUSED(proj);

	/* We use bounds() and xy2ll() here as this fallback implementation is
	   used ONLY for maps providing those functions since map creation. */
	QRectF b(bounds());
	double dx = b.width() / SAMPLES;
	double dy = b.height() / SAMPLES;

	Coordinates tl(xy2ll(b.topLeft()));
	Coordinates br(xy2ll(b.bottomRight()));
	RectC rect(tl, br);

	for (int i = 0; i <= SAMPLES; i++) {
		double x = b.left() + i * dx;
		growBottom(xy2ll(QPointF(x, b.bottom())), rect);
		growTop(xy2ll(QPointF(x, b.top())), rect);
	}

	for (int i = 0; i <= SAMPLES; i++) {
		double y = b.top() + i * dy;
		growLeft(xy2ll(QPointF(b.left(), y)), rect);
		growRight(xy2ll(QPointF(b.right(), y)), rect);
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
