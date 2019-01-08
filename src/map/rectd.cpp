#include "common/rectc.h"
#include "projection.h"
#include "rectd.h"


#define SAMPLE_POINTS 100

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

RectD::RectD(const RectC &rect, const Projection &proj)
{
	RectD prect;
	double dx = (rect.right() - rect.left()) / SAMPLE_POINTS;
	double dy = (rect.top() - rect.bottom()) / SAMPLE_POINTS;

	growRect(proj, rect.topLeft(), prect);

	if (dx > 0) {
		for (int i = 0; i <= SAMPLE_POINTS; i++) {
			double x = rect.left() + i * dx;
			growRect(proj, Coordinates(x, rect.bottom()), prect);
			growRect(proj, Coordinates(x, rect.top()), prect);
		}
	}
	if (dy > 0) {
		for (int i = 0; i <= SAMPLE_POINTS; i++ ) {
			double y = rect.bottom() + i * dy;
			growRect(proj, Coordinates(rect.left(), y), prect);
			growRect(proj, Coordinates(rect.right(), y), prect);
		}
	}

	*this = prect;
}
