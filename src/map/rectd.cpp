#include <cmath>
#include "common/rectc.h"
#include "projection.h"
#include "rectd.h"


static void growRect(const Projection &proj, const Coordinates &c, RectD &rect)
{
	PointD p(proj.ll2xy(c));

	if (p.x() < rect.left())
		rect.setLeft(p.x());
	if (p.x() > rect.right())
		rect.setRight(p.x());
	if (p.y() > rect.top())
		rect.setTop(p.y());
	if (p.y() < rect.bottom())
		rect.setBottom(p.y());
}

static void growRect(const Projection &proj, const PointD &p, RectC &rect)
{
	Coordinates c(proj.xy2ll(p));

	if (c.lon() < rect.left())
		rect.setLeft(c.lon());
	if (c.lon() > rect.right())
		rect.setRight(c.lon());
	if (c.lat() > rect.top())
		rect.setTop(c.lat());
	if (c.lat() < rect.bottom())
		rect.setBottom(c.lat());
}

RectD::RectD(const RectC &rect, const Projection &proj, int samples)
{
	if (!rect.isValid())
		return;

	double dx = rect.width() / samples;
	double dy = rect.height() / samples;

	PointD tl(proj.ll2xy(rect.topLeft()));
	PointD br(proj.ll2xy(rect.bottomRight()));
	RectD prect(tl, br);

	for (int i = 0; i <= samples; i++) {
		double x = remainder(rect.left() + i * dx, 360.0);
		growRect(proj, Coordinates(x, rect.bottom()), prect);
		growRect(proj, Coordinates(x, rect.top()), prect);
	}

	for (int i = 0; i <= samples; i++) {
		double y = rect.bottom() + i * dy;
		growRect(proj, Coordinates(rect.left(), y), prect);
		growRect(proj, Coordinates(rect.right(), y), prect);
	}

	*this = prect;
}

RectC RectD::toRectC(const Projection &proj, int samples) const
{
	if (!isValid())
		return RectC();

	double dx = width() / samples;
	double dy = height() / samples;

	Coordinates c(proj.xy2ll(center()));
	RectC rect(c, c);

	for (int i = 0; i <= samples; i++) {
		double x = left() + i * dx;
		growRect(proj, PointD(x, bottom()), rect);
		growRect(proj, PointD(x, top()), rect);
	}

	for (int i = 0; i <= samples; i++) {
		double y = bottom() + i * dy;
		growRect(proj, PointD(left(), y), rect);
		growRect(proj, PointD(right(), y), rect);
	}

	return rect;
}
