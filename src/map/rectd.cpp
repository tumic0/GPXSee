#include "common/rectc.h"
#include "projection.h"
#include "rectd.h"


static void growRect(const Projection &proj, const Coordinates &c, RectD &rect)
{
	if (c.isNull())
		return;

	PointD p(proj.ll2xy(c));

	if (rect.isNull())
		rect = RectD(p, p);
	else {
		if (p.x() < rect.left())
			rect.setLeft(p.x());
		if (p.x() > rect.right())
			rect.setRight(p.x());
		if (p.y() < rect.bottom())
			rect.setBottom(p.y());
		if (p.y() > rect.top())
			rect.setTop(p.y());
	}
}

static void growRect(const Projection &proj, const PointD &p, RectC &rect)
{
	if (p.isNull())
		return;

	Coordinates c(proj.xy2ll(p));

	if (rect.isNull())
		rect = RectC(c, c);
	else {
		if (c.lon() < rect.left())
			rect.setLeft(c.lon());
		if (c.lon() > rect.right())
			rect.setRight(c.lon());
		if (c.lat() < rect.bottom())
			rect.setBottom(c.lat());
		if (c.lat() > rect.top())
			rect.setTop(c.lat());
	}
}

RectD::RectD(const RectC &rect, const Projection &proj, int samples)
{
	RectD prect;
	double dx = (rect.right() - rect.left()) / samples;
	double dy = (rect.top() - rect.bottom()) / samples;

	growRect(proj, rect.topLeft(), prect);

	if (dx > 0) {
		for (int i = 0; i <= samples; i++) {
			double x = rect.left() + i * dx;
			growRect(proj, Coordinates(x, rect.bottom()), prect);
			growRect(proj, Coordinates(x, rect.top()), prect);
		}
	}
	if (dy > 0) {
		for (int i = 0; i <= samples; i++ ) {
			double y = rect.bottom() + i * dy;
			growRect(proj, Coordinates(rect.left(), y), prect);
			growRect(proj, Coordinates(rect.right(), y), prect);
		}
	}

	*this = prect;
}

RectC RectD::toRectC(const Projection &proj, int samples) const
{
	RectC ret;
	double dx = (right() - left()) / samples;
	double dy = (top() - bottom()) / samples;

	growRect(proj, topLeft(), ret);

	if (dx > 0) {
		for (int i = 0; i <= samples; i++) {
			double x = left() + i * dx;
			growRect(proj, PointD(x, bottom()), ret);
			growRect(proj, PointD(x, top()), ret);
		}
	}
	if (dy > 0) {
		for (int i = 0; i <= samples; i++ ) {
			double y = bottom() + i * dy;
			growRect(proj, PointD(left(), y), ret);
			growRect(proj, PointD(right(), y), ret);
		}
	}

	return ret;
}
