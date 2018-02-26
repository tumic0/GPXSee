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

#include <cmath>
#include "ellipsoid.h"
#include "lambertconic.h"

#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif // M_PI_2
#ifndef M_PI_4
	#define M_PI_4 0.785398163397448309616
#endif /* M_PI_4 */

#define LAMBERT_m(clat, essin) (clat / sqrt(1.0 - essin * essin))
#define LAMBERT2_t(lat, essin, es_over_2) \
	(tan(M_PI_4 - lat / 2) * pow((1.0 + essin) / (1.0 - essin), es_over_2))
#define LAMBERT1_t(lat, essin, es_over_2) \
	(tan(M_PI_4 - lat / 2) / pow((1.0 - essin) / (1.0 + essin), es_over_2))


LambertConic1::LambertConic1(const Ellipsoid *ellipsoid, double latitudeOrigin,
  double longitudeOrigin, double scale, double falseEasting,
  double falseNorthing)
{
	double es2;
	double es_sin;
	double m0;
	double lat_orig;


	lat_orig = deg2rad(latitudeOrigin);
	_longitudeOrigin = deg2rad(longitudeOrigin);
	if (_longitudeOrigin > M_PI)
		_longitudeOrigin -= 2 * M_PI;

	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;

	es2 = 2.0 * ellipsoid->flattening() - ellipsoid->flattening()
	  * ellipsoid->flattening();
	_es = sqrt(es2);
	_es_over_2 = _es / 2.0;

	_n = sin(lat_orig);

	es_sin = _es * sin(lat_orig);
	m0 = LAMBERT_m(cos(lat_orig), es_sin);
	_t0 = LAMBERT1_t(lat_orig, es_sin, _es_over_2);

	_rho0 = ellipsoid->radius() * scale * m0 / _n;

	_rho_olat = _rho0;
}

QPointF LambertConic1::ll2xy(const Coordinates &c) const
{
	double t;
	double rho;
	double dlam;
	double theta;
	double lat = deg2rad(c.lat());


	if (fabs(fabs(lat) - M_PI_2) > 1.0e-10) {
		t = LAMBERT1_t(lat, _es * sin(lat), _es_over_2);
		rho = _rho0 * pow(t / _t0, _n);
	} else
		rho = 0.0;

	dlam = deg2rad(c.lon()) - _longitudeOrigin;

	if (dlam > M_PI)
		dlam -= 2 * M_PI;
	if (dlam < -M_PI)
		dlam += 2 * M_PI;

	theta = _n * dlam;

	return QPointF(rho * sin(theta) + _falseEasting, _rho_olat - rho
	  * cos(theta) + _falseNorthing);
}

Coordinates LambertConic1::xy2ll(const QPointF &p) const
{
	double dx;
	double dy;
	double rho;
	double rho_olat_minus_dy;
	double t;
	double PHI;
	double es_sin;
	double tempPHI = 0.0;
	double theta = 0.0;
	double tolerance = 4.85e-10;
	int count = 30;
	double lat, lon;


	dy = p.y() - _falseNorthing;
	dx = p.x() - _falseEasting;
	rho_olat_minus_dy = _rho_olat - dy;
	rho = sqrt(dx * dx + (rho_olat_minus_dy) * (rho_olat_minus_dy));

	if (_n < 0.0) {
		rho *= -1.0;
		dx *= -1.0;
		rho_olat_minus_dy *= -1.0;
	}

	if (rho != 0.0) {
		theta = atan2(dx, rho_olat_minus_dy) / _n;
		t = _t0 * pow(rho / _rho0, 1 / _n);
		PHI = M_PI_2 - 2.0 * atan(t);
		while (fabs(PHI - tempPHI) > tolerance && count) {
			tempPHI = PHI;
			es_sin = _es * sin(PHI);
			PHI = M_PI_2 - 2.0 * atan(t * pow((1.0 - es_sin) / (1.0 + es_sin),
			  _es_over_2));
			count--;
		}

		if (!count)
			return Coordinates();

		lat = PHI;
		lon = theta + _longitudeOrigin;

		if (fabs(lat) < 2.0e-7)
			lat = 0.0;
		if (lat > M_PI_2)
			lat = M_PI_2;
		else if (lat < -M_PI_2)
			lat = -M_PI_2;

		if (lon > M_PI) {
			if (lon - M_PI < 3.5e-6)
				lon = M_PI;
			else
				lon -= 2 * M_PI;
		}
		if (lon < -M_PI) {
			if (fabs(lon + M_PI) < 3.5e-6)
				lon = -M_PI;
			else
				lon += 2 * M_PI;
		}

		if (fabs(lon) < 2.0e-7)
			lon = 0.0;
		if (lon > M_PI)
			lon = M_PI;
		else if (lon < -M_PI)
			lon = -M_PI;
	} else {
		if (_n > 0.0)
			lat = M_PI_2;
		else
			lat = -M_PI_2;
		lon = _longitudeOrigin;
	}

	return Coordinates(rad2deg(lon), rad2deg(lat));
}

LambertConic2::LambertConic2(const Ellipsoid *ellipsoid,
  double standardParallel1, double standardParallel2, double latitudeOrigin,
  double longitudeOrigin, double falseEasting, double falseNorthing)
{
	double es, es_over_2, es2, es_sin;
	double lat0;
	double k0;
	double t0;
	double t1, t2;
	double t_olat;
	double m0;
	double m1;
	double m2;
	double n;
	double const_value;
	double sp1, sp2;
	double lat_orig;


	lat_orig = deg2rad(latitudeOrigin);
	sp1 = deg2rad(standardParallel1);
	sp2 = deg2rad(standardParallel2);

	if (fabs(sp1 - sp2) > 1.0e-10) {
		es2 = 2 * ellipsoid->flattening() - ellipsoid->flattening()
		  * ellipsoid->flattening();
		es = sqrt(es2);
		es_over_2 = es / 2.0;

		es_sin = es * sin(lat_orig);
		t_olat = LAMBERT2_t(lat_orig, es_sin, es_over_2);

		es_sin = es * sin(sp1);
		m1 = LAMBERT_m(cos(sp1), es_sin);
		t1 = LAMBERT2_t(sp1, es_sin, es_over_2);

		es_sin = es * sin(sp2);
		m2 = LAMBERT_m(cos(sp2), es_sin);
		t2 = LAMBERT2_t(sp2, es_sin, es_over_2);

		n = log(m1 / m2) / log(t1 / t2);

		lat0 = asin(n);

		es_sin = es * sin(lat0);
		m0 = LAMBERT_m(cos(lat0), es_sin);
		t0 = LAMBERT2_t(lat0, es_sin, es_over_2);

		k0 = (m1 / m0) * (pow(t0 / t1, n));

		const_value = ((ellipsoid->radius() * m2) / (n * pow(t2, n)));

		falseNorthing += (const_value * pow(t_olat, n)) - (const_value
		  * pow(t0, n));
	} else {
		lat0 = sp1;
		k0 = 1.0;
	}

	_lc1 = LambertConic1(ellipsoid, rad2deg(lat0), longitudeOrigin, k0,
	  falseEasting, falseNorthing);
}

QPointF LambertConic2::ll2xy(const Coordinates &c) const
{
	return _lc1.ll2xy(c);
}

Coordinates LambertConic2::xy2ll(const QPointF &p) const
{
	return _lc1.xy2ll(p);
}
