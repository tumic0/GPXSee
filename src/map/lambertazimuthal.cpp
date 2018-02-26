#include <cmath>
#include "ellipsoid.h"
#include "lambertazimuthal.h"

#define sin2(x) (sin(x) * sin(x))
#define sqr(x) ((x) * (x))

LambertAzimuthal::LambertAzimuthal(const Ellipsoid *ellipsoid,
  double latitudeOrigin, double longitudeOrigin, double falseEasting,
  double falseNorthing)
{
	double lat0 = deg2rad(latitudeOrigin);

	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;
	_lon0 = deg2rad(longitudeOrigin);

	_a = ellipsoid->radius();

	_es2 = 2.0 * ellipsoid->flattening() - ellipsoid->flattening()
	  * ellipsoid->flattening();
	_es = sqrt(_es2);

	double q0 = (1.0 - _es2) * ((sin(lat0) / (1.0 - _es2 * sin2(lat0)))
	  - ((1.0/(2.0*_es)) * log((1.0 - _es * sin(lat0)) / (1.0 + _es
	  * sin(lat0)))));
	_qP = (1.0 - _es2) * ((1.0 / (1.0 - _es2)) - ((1.0/(2.0*_es))
	  * log((1.0 - _es) / (1.0 + _es))));
	_beta0 = asin(q0 / _qP);
	_Rq = _a * sqrt(_qP / 2.0);
	_D = _a * (cos(lat0) / sqrt(1.0 - _es2 * sin2(lat0))) / (_Rq * cos(_beta0));
}

QPointF LambertAzimuthal::ll2xy(const Coordinates &c) const
{
	double lon = deg2rad(c.lon());
	double lat = deg2rad(c.lat());

	double q = (1.0 - _es2) * ((sin(lat) / (1.0 - _es2 * sin2(lat)))
	  - ((1.0/(2.0*_es)) * log((1.0 - _es * sin(lat)) / (1.0 + _es
	  * sin(lat)))));
	double beta = asin(q / _qP);
	double B = _Rq * sqrt(2.0 / (1.0 + sin(_beta0) * sin(beta) + (cos(_beta0)
	  * cos(beta) * cos(lon - _lon0))));

	double x = _falseEasting + ((B * _D) * (cos(beta) * sin(lon - _lon0)));
	double y = _falseNorthing + (B / _D) * ((cos(_beta0) * sin(beta))
	  - (sin(_beta0) * cos(beta) * cos(lon - _lon0)));

	return QPointF(x, y);
}

Coordinates LambertAzimuthal::xy2ll(const QPointF &p) const
{
	double es4 = _es2 * _es2;
	double es6 = _es2 * es4;

	double rho = sqrt(sqr((p.x() - _falseEasting) / _D) + sqr(_D * (p.y()
	  - _falseNorthing)));
	double C = 2.0 * asin(rho / (2.0*_Rq));
	double betaS = asin((cos(C) * sin(_beta0)) + ((_D * (p.y() -_falseNorthing)
	  * sin(C) * cos(_beta0)) / rho));

	double lon = _lon0 + atan((p.x() - _falseEasting) * sin(C) / (_D * rho
	  * cos(_beta0) * cos(C) - sqr(_D) * (p.y() - _falseNorthing) * sin(_beta0)
	  * sin(C)));
	double lat = betaS + ((_es2/3.0 + 31.0*es4/180.0 + 517.0*es6/5040.0)
	  * sin(2.0*betaS)) + ((23.0*es4/360.0 + 251.0*es6/3780.0) * sin(4.0*betaS))
	  + ((761.0*es6/45360.0)*sin(6.0*betaS));

	return Coordinates(rad2deg(lon), rad2deg(lat));
}
