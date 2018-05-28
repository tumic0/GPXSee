#include "ellipsoid.h"
#include "krovak.h"

Krovak::Krovak(const Ellipsoid *ellipsoid, double standardParallel,
  double azimuth, double scale, double centerLatitude, double longitudeOrigin,
  double falseEasting, double falseNorthing, Orientation orientation)
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
	_A = ellipsoid->radius() * sqrt(1.0 - ellipsoid->es())
	  / (1.0 - ellipsoid->es() * sinPhiC2);
	_B = sqrt(1.0 + (ellipsoid->es() * cosPhiC4 / (1.0 - ellipsoid->es())));
	double gamma0 = asin(sinPhiC / _B);
	_t0 = tan(M_PI_4 + gamma0 / 2.0) * pow((1.0 + _e * sinPhiC) /
	  (1.0 - _e * sinPhiC), _e*_B / 2.0) / pow(tan(M_PI_4 + phiC/2.0), _B);
	_n = sin(_phiP);
	_r0 = scale * _A / tan(_phiP);
	_FE = falseEasting;
	_FN = falseNorthing;
	_cosAlphaC = cos(alphaC);
	_sinAlphaC = sin(alphaC);
	_lambda0 = deg2rad(longitudeOrigin);
	_orientation = orientation;
}

PointD Krovak::ll2xy(const Coordinates &c) const
{
	double phi = deg2rad(c.lat());
	double lambda = deg2rad(c.lon());
	double sinPhi = sin(phi);
	double eSinPhi = _e * sinPhi;

	double U = 2.0 * (atan(_t0 * pow(tan(phi/2.0 + M_PI_4), _B)
	  / pow((1.0 + eSinPhi) / (1.0 - eSinPhi), _e * _B / 2.0)) - M_PI_4);
	double V = _B * (_lambda0 - lambda);
	double T = asin(_cosAlphaC * sin(U) + _sinAlphaC * cos(U) * cos(V));
	double D = asin(cos(U) * sin(V) / cos(T));
	double theta = _n * D;
	double r = _r0 * pow(tan(M_PI_4 + _phiP / 2.0), _n)
	  / pow(tan(T/2.0 + M_PI_4), _n);
	double sign = (_orientation == North) ? -1.0 : 1.0;

	return PointD(sign * (r * sin(theta) + _FE), sign * (r * cos(theta) + _FN));
}

Coordinates Krovak::xy2ll(const PointD &p) const
{
	double sign = (_orientation == North) ? -1.0 : 1.0;
	double Xp = sign * p.y() - _FN;
	double Yp = sign * p.x() - _FE;
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
		phi = 2.0 * (atan(pow(_t0, -1.0/_B) * pow(tan(U/2.0 + M_PI_4), 1.0/_B)
		  * pow((1.0 + _e * sin(phi))/(1.0 - _e * sin(phi)), _e/2.0)) - M_PI_4);

	return Coordinates(rad2deg(_lambda0 - V/_B), rad2deg(phi));
}
