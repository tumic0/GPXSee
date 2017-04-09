#include <cmath>
#include "rd.h"
#include "lambertconic.h"

#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif // M_PI_2
#ifndef M_PI_4
	#define M_PI_4 0.78539816339744830962
#endif // M_PI_4

static double q(const Ellipsoid &el, double b)
{
	double e = sqrt(el.flattening() * (2. - el.flattening()));
	double esb = e * sin(b);
	return log(tan(M_PI_4 + b / 2.) * pow((1. - esb) / (1. + esb), e / 2.));
}

static double iq(const Ellipsoid &el, double q)
{
	double e = sqrt(el.flattening() * (2. - el.flattening()));
	double b0 = 0.;
	double b = 2. * atan(exp(q)) - M_PI_2;

	do {
		b0 = b;
		double esb = e * sin(b);
		b = 2. * atan(exp(q) * pow((1. - esb) / (1. + esb), -e / 2.)) - M_PI_2;
	} while (fabs(b - b0) > 1e-10);

	return b;
}

static double nradius(const Ellipsoid &el, double phi)
{
	double sin_phi = sin(phi);
	return (el.radius() / sqrt(1. - (el.flattening() * (2. - el.flattening()))
	  * sin_phi * sin_phi));
}

LambertConic::LambertConic(const Ellipsoid &ellipsoid, double standardParallel1,
  double standardParallel2, double latitudeOrigin, double longitudeOrigin,
  double scale, double falseEasting, double falseNorthing) : _e(ellipsoid)
{
	_cm = longitudeOrigin;
	_fe = falseEasting;
	_fn = falseNorthing;

	double sp1 = deg2rad(standardParallel1);
	double sp2 = deg2rad(standardParallel2);

	double N1 = nradius(_e, sp1);
	double N2 = nradius(_e, sp2);

	_q0 = q(_e, deg2rad(latitudeOrigin));
	double q1 = q(_e, sp1);
	double q2 = q(_e, sp2);

	_n = log((N1 * cos(sp1)) / (N2 * cos(sp2))) / (q2 - q1);
	double R1 = N1 * cos(sp1) / _n;
	_R0 = scale * R1 * exp(_n * (q1 - _q0));
}

QPointF LambertConic::ll2xy(const Coordinates &c) const
{
	double dl = _n * (deg2rad(c.lon()) - deg2rad(_cm));
	double R = _R0 * exp(_n * (_q0 - q(_e, deg2rad(c.lat()))));

	return QPointF(_fe + R * sin(dl), _fn + _R0 - R * cos(dl));
}

Coordinates LambertConic::xy2ll(const QPointF &p) const
{
	double dl = atan((p.x() - _fe) / (_fn + _R0 - p.y()));
	double dx = p.x() - _fe;
	double dy = p.y() - _fn - _R0;
	double R = sqrt(dx * dx + dy * dy);
	double q = _q0 - log(R / _R0) / _n;

	return Coordinates(rad2deg(deg2rad(_cm) + dl / _n), rad2deg(iq(_e, q)));
}
