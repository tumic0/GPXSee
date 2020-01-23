#include "ellipsoid.h"
#include "obliquestereographic.h"


#define S1(x) ((1.0 + x) / (1.0 - x))
#define S2(x) ((1.0 - _e * x) / (1.0 + _e * x))

ObliqueStereographic::ObliqueStereographic(const Ellipsoid *ellipsoid,
  double latitudeOrigin, double longitudeOrigin, double scale,
  double falseEasting, double falseNorthing)
  : _fe(falseEasting), _fn(falseNorthing)
{
	double lat0 = deg2rad(latitudeOrigin);
	double sinPhi0 = sin(lat0);
	double cosPhi0 = cos(lat0);
	double sinLat0s = sinPhi0 * sinPhi0;

	double a = ellipsoid->radius();
	_es = ellipsoid->es();
	_e = sqrt(_es);

	double rho0 = a * (1.0 - _es) / pow(1.0 - _es * sinLat0s, 1.5);
	double nu0 = a / sqrt(1.0 - _es * sinLat0s);

	_n = sqrt(1.0 + ((_es * pow(cosPhi0, 4)) / (1.0 - _es)));
	double w1 = pow(S1(sinPhi0) * pow(S2(sinPhi0), _e), _n);
	double sinChi0 = (w1 - 1) / (w1 + 1);
	_c = (_n + sinPhi0) * (1.0 - sinChi0) / ((_n - sinPhi0) * (1.0 + sinChi0));
	double w2 = _c * w1;

	_chi0 = asin((w2 - 1.0)/(w2 + 1.0));
	_sinChi0 = sin(_chi0); _cosChi0 = cos(_chi0);
	_lambda0 = deg2rad(longitudeOrigin);
	_twoRk0 = 2.0 * sqrt(rho0 * nu0) * scale;
	_g = _twoRk0 * tan(M_PI_4 - _chi0 / 2.0);
	_h = 2.0 * _twoRk0 * tan(_chi0) + _g;
}

PointD ObliqueStereographic::ll2xy(const Coordinates &c) const
{
	double lon = deg2rad(c.lon());
	double lat = deg2rad(c.lat());

	double sinPhi = sin(lat);
	double w = _c * pow(S1(sinPhi) * pow(S2(sinPhi), _e), _n);
	double chi = asin((w - 1.0) / (w + 1.0));
	double lambda = _n * (lon - _lambda0) + _lambda0;
	double B = (1.0 + sin(chi) * _sinChi0 + cos(chi) * _cosChi0
	  * cos(lambda - _lambda0));

	return PointD(_fe + _twoRk0 * cos(chi) * sin(lambda - _lambda0) / B,
	  _fn + _twoRk0 * (sin(chi) * _cosChi0 - cos(chi) * _sinChi0
	  * cos(lambda - _lambda0)) / B);
}

Coordinates ObliqueStereographic::xy2ll(const PointD &p) const
{
	double i = atan((p.x() - _fe) / (_h + (p.y() - _fn)));
	double j = atan((p.x() - _fe) / (_g - (p.y() - _fn))) - i;

	double chi = _chi0 + 2.0 * atan(((p.y() - _fn) - (p.x() - _fe) * tan(j/2.0))
	  / _twoRk0);
	double lambda = j + 2.0 * i + _lambda0;

	double psi0 = 0.5 * log((1.0 + sin(chi)) / (_c * (1.0 - sin(chi)))) / _n;
	double phi1 = 2.0 * atan(pow(M_E, psi0)) - M_PI_2;

	double psi, phi = phi1, prev = phi;
	for (int i = 0; i < 8; i++) {
		double sinPhi = sin(phi);
		psi = log((tan(phi/2.0 + M_PI_4)) * pow((1.0 - _e * sinPhi)
		  / (1.0 + _e * sinPhi), _e/2.0));
		phi = phi - (psi - psi0) * cos(phi) * (1.0 - _es * sinPhi * sinPhi)
		  / (1.0 - _es);
		if (fabs(phi - prev) < 1e-10)
			break;
		prev = phi;
	}

	return Coordinates(rad2deg((lambda - _lambda0) / _n + _lambda0),
	  rad2deg(phi));
}
