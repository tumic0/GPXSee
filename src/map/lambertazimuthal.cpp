#include <cmath>
#include "ellipsoid.h"
#include "lambertazimuthal.h"

#define sin2(x) (sin(x) * sin(x))
#define sqr(x) ((x) * (x))

LambertAzimuthal::LambertAzimuthal(const Ellipsoid &ellipsoid,
  double latitudeOrigin, double longitudeOrigin, double falseEasting,
  double falseNorthing)
{
	double lat0 = deg2rad(latitudeOrigin);

	_fe = falseEasting;
	_fn = falseNorthing;
	_lon0 = deg2rad(longitudeOrigin);

	_a = ellipsoid.radius();
	_es = ellipsoid.es();
	_e = sqrt(_es);

	double q0 = (1.0 - _es) * ((sin(lat0) / (1.0 - _es * sin2(lat0)))
	  - ((1.0/(2.0*_e)) * log((1.0 - _e * sin(lat0)) / (1.0 + _e
	  * sin(lat0)))));
	_qP = (1.0 - _es) * ((1.0 / (1.0 - _es)) - ((1.0/(2.0*_e))
	  * log((1.0 - _e) / (1.0 + _e))));
	_beta0 = asin(q0 / _qP);
	_rq = _a * sqrt(_qP / 2.0);
	_d = _a * (cos(lat0) / sqrt(1.0 - _es * sin2(lat0))) / (_rq * cos(_beta0));
}

PointD LambertAzimuthal::ll2xy(const Coordinates &c) const
{
	double lon = deg2rad(c.lon());
	double lat = deg2rad(c.lat());

	double q = (1.0 - _es) * ((sin(lat) / (1.0 - _es * sin2(lat)))
	  - ((1.0/(2.0*_e)) * log((1.0 - _e * sin(lat)) / (1.0 + _e
	  * sin(lat)))));
	double beta = asin(q / _qP);
	double B = _rq * sqrt(2.0 / (1.0 + sin(_beta0) * sin(beta) + (cos(_beta0)
	  * cos(beta) * cos(lon - _lon0))));

	double x = _fe + ((B * _d) * (cos(beta) * sin(lon - _lon0)));
	double y = _fn + (B / _d) * ((cos(_beta0) * sin(beta))
	  - (sin(_beta0) * cos(beta) * cos(lon - _lon0)));

	return PointD(x, y);
}

Coordinates LambertAzimuthal::xy2ll(const PointD &p) const
{
	double es4 = _es * _es;
	double es6 = _es * es4;

	double rho = sqrt(sqr((p.x() - _fe) / _d) + sqr(_d * (p.y()
	  - _fn)));
	double C = 2.0 * asin(rho / (2.0*_rq));
	double betaS = asin((cos(C) * sin(_beta0)) + ((_d * (p.y() -_fn)
	  * sin(C) * cos(_beta0)) / rho));

	double lon = _lon0 + atan((p.x() - _fe) * sin(C) / (_d * rho
	  * cos(_beta0) * cos(C) - sqr(_d) * (p.y() - _fn) * sin(_beta0)
	  * sin(C)));
	double lat = betaS + ((_es/3.0 + 31.0*es4/180.0 + 517.0*es6/5040.0)
	  * sin(2.0*betaS)) + ((23.0*es4/360.0 + 251.0*es6/3780.0) * sin(4.0*betaS))
	  + ((761.0*es6/45360.0)*sin(6.0*betaS));

	return Coordinates(rad2deg(lon), rad2deg(lat));
}

bool LambertAzimuthal::operator==(const CT &ct) const
{
	const LambertAzimuthal *other = dynamic_cast<const LambertAzimuthal*>(&ct);
	return (other != 0 && _lon0 == other->_lon0 && _fn == other->_fn
	  && _fe == other->_fe && _a == other->_a && _es == other->_es
	  && _beta0 == other->_beta0 && _rq == other->_rq && _d == other->_d);
}
