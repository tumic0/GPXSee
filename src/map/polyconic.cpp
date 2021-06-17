/*
 * Based on libgeotrans with the following Source Code Disclaimer:

1. The GEOTRANS source code ("the software") is provided free of charge by
the National Imagery and Mapping Agency (NIMA) of the United States
Department of Defense. Although NIMA makes no copyright claim under Title 17
U.S.C., NIMA claims copyrights in the source code under other legal regimes.
NIMA hereby grants to each user of the software a license to use and
distribute the software, and develop derivative works.

2. Warranty Disclaimer: The software was developed to meet only the internal
requirements of the U.S. National Imagery and Mapping Agency. The software
is provided "as is," and no warranty, express or implied, including but not
limited to the implied warranties of merchantability and fitness for
particular purpose or arising by statute or otherwise in law or from a
course of dealing or usage in trade, is made by NIMA as to the accuracy and
functioning of the software.

3. NIMA and its personnel are not required to provide technical support or
general assistance with respect to the software.

4. Neither NIMA nor its personnel will be liable for any claims, losses, or
damages arising from or connected with the use of the software. The user
agrees to hold harmless the United States National Imagery and Mapping
Agency. The user's sole and exclusive remedy is to stop using the software.

5. NIMA requests that products developed using the software credit the
source of the software with the following statement, "The product was
developed using GEOTRANS, a product of the National Imagery and Mapping
Agency and U.S. Army Engineering Research and Development Center."

6. For any products developed using the software, NIMA requires a disclaimer
that use of the software does not indicate endorsement or approval of the
product by the Secretary of Defense or the National Imagery and Mapping
Agency. Pursuant to the United States Code, 10 U.S.C. Sec. 2797, the name of
the National Imagery and Mapping Agency, the initials "NIMA", the seal of
the National Imagery and Mapping Agency, or any colorable imitation thereof
shall not be used to imply approval, endorsement, or authorization of a
product without prior written permission from United States Secretary of
Defense.

*/

#include "ellipsoid.h"
#include "polyconic.h"

#define POLY_COEFF_TIMES_SIN(coeff, x, latit) \
	(coeff * (sin (x * latit)))
#define POLY_M(c0lat, c1s2lat, c2s4lat, c3s6lat) \
	(_a * (c0lat - c1s2lat + c2s4lat - c3s6lat))
#define FLOAT_EQ(x, v, epsilon) \
	(((v - epsilon) < x) && (x < (v + epsilon)))

Polyconic::Polyconic(const Ellipsoid &ellipsoid, double latitudeOrigin,
  double longitudeOrigin, double falseEasting, double falseNorthing)
{
	double j, three_es4;
	double lat, sin2lat, sin4lat, sin6lat;
	double a2;
	double b2;

	_longitudeOrigin = deg2rad(longitudeOrigin);
	_latitudeOrigin = deg2rad(latitudeOrigin);
	_a = ellipsoid.radius();
	_b = ellipsoid.b();
	if (_longitudeOrigin > M_PI)
		_longitudeOrigin -= 2 * M_PI;
	_falseNorthing = falseNorthing;
	_falseEasting = falseEasting;
	a2 = _a * _a;
	b2 = _b * _b;
	_es2 = (a2 - b2) / a2;
	_es4 = _es2 * _es2;
	_es6 = _es4 * _es2;

	j = 45.0 * _es6 / 1024.0;
	three_es4 = 3.0 * _es4;
	_c0 = 1.0 - _es2 / 4.0 - three_es4 / 64.0 - 5.0 * _es6 / 256.0;
	_c1 = 3.0 * _es2 / 8.0 + three_es4 / 32.0 + j;
	_c2 = 15.0 * _es4 / 256.0 + j;
	_c3 = 35.0 * _es6 / 3072.0;

	lat = _c0 * _latitudeOrigin;
	sin2lat = POLY_COEFF_TIMES_SIN(_c1, 2.0, _latitudeOrigin);
	sin4lat = POLY_COEFF_TIMES_SIN(_c2, 4.0, _latitudeOrigin);
	sin6lat = POLY_COEFF_TIMES_SIN(_c3, 6.0, _latitudeOrigin);
	_M0 = POLY_M(lat, sin2lat, sin4lat, sin6lat);
}

PointD Polyconic::ll2xy(const Coordinates &c) const
{
	double Longitude = deg2rad(c.lon());
	double Latitude = deg2rad(c.lat());
	double slat = sin(Latitude);
	double lat, sin2lat, sin4lat, sin6lat;
	double dlam;
	double NN;
	double NN_OVER_tlat;
	double MM;
	double EE;

	dlam = Longitude - _longitudeOrigin;
	if (dlam > M_PI)
		dlam -= 2 * M_PI;
	if (dlam < -M_PI)
		dlam += 2 * M_PI;

	if (Latitude == 0.0) {
		return PointD(_a * dlam + _falseEasting,
		  -_M0 + _falseNorthing);
	} else {
		NN = _a / sqrt(1.0 - _es2 * (slat * slat));
		NN_OVER_tlat = NN  / tan(Latitude);
		lat = _c0 * Latitude;
		sin2lat = POLY_COEFF_TIMES_SIN(_c1, 2.0, Latitude);
		sin4lat = POLY_COEFF_TIMES_SIN(_c2, 4.0, Latitude);
		sin6lat = POLY_COEFF_TIMES_SIN(_c3, 6.0, Latitude);
		MM = POLY_M(lat, sin2lat, sin4lat, sin6lat);
		EE = dlam * slat;
		return PointD(NN_OVER_tlat * sin(EE) + _falseEasting,
		  MM - _M0 + NN_OVER_tlat * (1.0 - cos(EE)) + _falseNorthing);
	}
}

Coordinates Polyconic::xy2ll(const PointD &p) const
{
	double dx;
	double dy;
	double dx_OVER_Poly_a;
	double AA;
	double BB;
	double CC = 0.0;
	double PHIn, Delta_PHI = 1.0;
	double sin_PHIn;
	double PHI, sin2PHI, sin4PHI, sin6PHI;
	double Mn, Mn_prime, Ma;
	double AA_Ma;
	double Ma2_PLUS_BB;
	double AA_MINUS_Ma;
	double tolerance = 1.0e-12;
	double Latitude;
	double Longitude;


	dy = p.y() - _falseNorthing;
	dx = p.x() - _falseEasting;
	dx_OVER_Poly_a = dx / _a;

	if (FLOAT_EQ(dy,-_M0,1)) {
		Latitude = 0.0;
		Longitude = dx_OVER_Poly_a + _longitudeOrigin;
	} else {
		AA = (_M0 + dy) / _a;
		BB = dx_OVER_Poly_a * dx_OVER_Poly_a + (AA * AA);
		PHIn = AA;

		while (fabs(Delta_PHI) > tolerance) {
			sin_PHIn = sin(PHIn);
			CC = sqrt(1.0 - _es2 * sin_PHIn * sin_PHIn) * tan(PHIn);
			PHI = _c0 * PHIn;
			sin2PHI = POLY_COEFF_TIMES_SIN(_c1, 2.0, PHIn);
			sin4PHI = POLY_COEFF_TIMES_SIN(_c2, 4.0, PHIn);
			sin6PHI = POLY_COEFF_TIMES_SIN(_c3, 6.0, PHIn);
			Mn = POLY_M(PHI, sin2PHI, sin4PHI, sin6PHI);
			Mn_prime = _c0 - 2.0 * _c1 * cos(2.0 * PHIn) + 4.0 * _c2
			  * cos(4.0 * PHIn) - 6.0 * _c3 * cos(6.0 * PHIn);
			Ma = Mn / _a;
			AA_Ma = AA * Ma;
			Ma2_PLUS_BB = Ma * Ma + BB;
			AA_MINUS_Ma = AA - Ma;
			Delta_PHI = (AA_Ma * CC + AA_MINUS_Ma - 0.5 * (Ma2_PLUS_BB) * CC) /
			  (_es2 * sin2PHI * (Ma2_PLUS_BB - 2.0 * AA_Ma) / 4.0 * CC
			   + (AA_MINUS_Ma) * (CC * Mn_prime - 2.0 / sin2PHI) - Mn_prime);
			PHIn -= Delta_PHI;
		}
		Latitude = PHIn;

		if (Latitude > M_PI_2)
			Latitude = M_PI_2;
		else if (Latitude < -M_PI_2)
			Latitude = -M_PI_2;

		if (FLOAT_EQ(fabs(Latitude), M_PI_2, 0.00001) || (Latitude == 0))
			Longitude = _longitudeOrigin;
		else
			Longitude = (asin(dx_OVER_Poly_a * CC)) / sin(Latitude)
			  + _longitudeOrigin;
	}

	if (Longitude > M_PI)
		Longitude -= 2 * M_PI;
	if (Longitude < -M_PI)
		Longitude += 2 *M_PI;

	if (Longitude > M_PI)
		Longitude = M_PI;
	else if (Longitude < -M_PI)
		Longitude = -M_PI;

	return Coordinates(rad2deg(Longitude), rad2deg(Latitude));
}

bool Polyconic::operator==(const CT &ct) const
{
	const Polyconic *other = dynamic_cast<const Polyconic*>(&ct);
	return (other != 0 && _a == other->_a && _b == other->_b
	  && _latitudeOrigin == other->_latitudeOrigin
	  && _longitudeOrigin == other->_longitudeOrigin
	  && _falseNorthing == other->_falseNorthing
	  && _falseEasting == other->_falseEasting);
}
