#include "ellipsoid.h"
#include "krovak.h"

Krovak::Krovak(const Ellipsoid *ellipsoid, double standardParallel,
  double azimuth, double scale, double centerLatitude, double longitudeOrigin,
  double falseEasting, double falseNorthing)
{
	double phiC = deg2rad(centerLatitude);
	double sinPhiC = sin(phiC);
	double sinPhiC2 = sinPhiC * sinPhiC;
	double cosPhiC = cos(phiC);
	double cosPhiC2 = cosPhiC * cosPhiC;
	double cosPhiC4 = cosPhiC2 * cosPhiC2;
	double alphaC = deg2rad(azimuth);

	_phiP = deg2rad(standardParallel);
	_e = sqrt(ellipsoid->es());
	_a = ellipsoid->radius() * sqrt(1.0 - ellipsoid->es())
	  / (1.0 - ellipsoid->es() * sinPhiC2);
	_b = sqrt(1.0 + (ellipsoid->es() * cosPhiC4 / (1.0 - ellipsoid->es())));
	double gamma0 = asin(sinPhiC / _b);
	_t0 = tan(M_PI_4 + gamma0 / 2.0) * pow((1.0 + _e * sinPhiC) /
	  (1.0 - _e * sinPhiC), _e*_b / 2.0) / pow(tan(M_PI_4 + phiC/2.0), _b);
	_n = sin(_phiP);
	_r0 = scale * _a / tan(_phiP);
	_fe = falseEasting;
	_fn = falseNorthing;
	_cosAlphaC = cos(alphaC);
	_sinAlphaC = sin(alphaC);
	_lambda0 = deg2rad(longitudeOrigin);
}

PointD Krovak::ll2xy(const Coordinates &c) const
{
	double phi = deg2rad(c.lat());
	double lambda = deg2rad(c.lon());
	double eSinPhi = _e * sin(phi);
	double U = 2.0 * (atan(_t0 * pow(tan(phi/2.0 + M_PI_4), _b)
	  / pow((1.0 + eSinPhi) / (1.0 - eSinPhi), _e * _b/2.0)) - M_PI_4);
	double cosU = cos(U);
	double V = _b * (_lambda0 - lambda);
	double T = asin(_cosAlphaC * sin(U) + _sinAlphaC * cosU * cos(V));
	double D = asin(cosU * sin(V) / cos(T));
	double theta = _n * D;
	double r = _r0 * pow(tan(M_PI_4 + _phiP/2.0), _n)
	  / pow(tan(T/2.0 + M_PI_4), _n);

	return PointD(r * sin(theta) + _fe, r * cos(theta) + _fn);
}

Coordinates Krovak::xy2ll(const PointD &p) const
{
	double Xp = p.y() - _fn;
	double Yp = p.x() - _fe;
	double Xp2 = Xp * Xp;
	double Yp2 = Yp * Yp;
	double r = sqrt(Xp2 + Yp2);
	double theta = atan(Yp / Xp);
	double D = theta / sin(_phiP);
	double T = 2.0 * (atan(pow(_r0 / r, 1.0/_n) * tan(M_PI_4 + _phiP/2.0))
	  - M_PI_4);
	double U = asin(_cosAlphaC * sin(T) - _sinAlphaC * cos(T) * cos(D));
	double V = asin(cos(T) * sin(D) / cos(U));
	double phi = U;
	for (int i = 0; i < 3; i++)
		phi = 2.0 * (atan(pow(_t0, -1.0/_b) * pow(tan(U/2.0 + M_PI_4), 1.0/_b)
		  * pow((1.0 + _e * sin(phi))/(1.0 - _e * sin(phi)), _e/2.0)) - M_PI_4);

	return Coordinates(rad2deg(_lambda0 - V/_b), rad2deg(phi));
}
