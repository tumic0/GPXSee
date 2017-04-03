#include <cmath>
#include "rd.h"
#include "lambertconic.h"


LambertConic::LambertConic(const Ellipsoid &ellipsoid, double standardParallel1,
  double standardParallel2, double centralParallel, double centralMeridian,
  double scale, double falseEasting, double falseNorthing) : _e(ellipsoid)
{
	_cm = centralMeridian;
	_fe = falseEasting;
	_fn = falseNorthing;

	double sp1 = deg2rad(standardParallel1);
	double sp2 = deg2rad(standardParallel2);

	double N1 = _e.nradius(sp1);
	double N2 = _e.nradius(sp2);

	_q0 = ellipsoid.q(deg2rad(centralParallel));
	double q1 = _e.q(sp1);
	double q2 = _e.q(sp2);

	_n = log((N1 * cos(sp1)) / (N2 * cos(sp2))) / (q2 - q1);
	double R1 = N1 * cos(sp1) / _n;
	_R0 = scale * R1 * exp(_n * (q1 - _q0));
}

QPointF LambertConic::ll2xy(const Coordinates &c) const
{
	double dl = _n * (deg2rad(c.lon()) - deg2rad(_cm));
	double R = _R0 * exp(_n * (_q0 - _e.q(deg2rad(c.lat()))));

	return QPointF(_fe + R * sin(dl), _fn + _R0 - R * cos(dl));
}

Coordinates LambertConic::xy2ll(const QPointF &p) const
{
	double dl = atan((p.x() - _fe) / (_fn + _R0 - p.y()));
	double dx = p.x() - _fe;
	double dy = p.y() - _fn - _R0;
	double R = sqrt(dx * dx + dy * dy);
	double q = _q0 - log(R / _R0) / _n;

	return Coordinates(rad2deg(deg2rad(_cm) + dl / _n), rad2deg(_e.iq(q)));
}
