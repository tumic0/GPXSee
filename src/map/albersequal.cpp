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
#include "albersequal.h"


#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif // M_PI_2

#define ONE_MINUS_SQR(x) (1.0 - (x) * (x))
#define ALBERS_Q(slat, one_minus_sqr_es_sin, es_sin) \
	(_one_minus_es2 * ((slat) / (one_minus_sqr_es_sin) - \
	(1 / (_two_es)) * log((1 - (es_sin)) / (1 + (es_sin)))))
#define ALBERS_M(clat, one_minus_sqr_es_sin) \
	((clat) / sqrt(one_minus_sqr_es_sin))


AlbersEqual::AlbersEqual(const Ellipsoid *ellipsoid, double standardParallel1,
  double standardParallel2, double latitudeOrigin, double longitudeOrigin,
  double falseEasting, double falseNorthing)
{
	double sin_lat, sin_lat1, sin_lat2, cos_lat1, cos_lat2;
	double m1, m2, sqr_m1, sqr_m2;
	double q0, q1, q2;
	double es_sin, es_sin1, es_sin2;
	double one_minus_sqr_es_sin1, one_minus_sqr_es_sin2;
	double nq0;
	double sp1, sp2;


	_latitudeOrigin = deg2rad(latitudeOrigin);
	_longitudeOrigin = deg2rad(longitudeOrigin);
	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;

	sp1 = deg2rad(standardParallel1);
	sp2 = deg2rad(standardParallel2);

	_a2 = ellipsoid->radius() * ellipsoid->radius();
	_es2 = 2 * ellipsoid->flattening() - ellipsoid->flattening()
	  * ellipsoid->flattening();
	_es = sqrt(_es2);
	_one_minus_es2 = 1 - _es2;
	_two_es = 2 * _es;

	sin_lat = sin(_latitudeOrigin);
	es_sin = _es * sin_lat;
	q0 = ALBERS_Q(sin_lat, ONE_MINUS_SQR(es_sin), es_sin);

	sin_lat1 = sin(sp1);
	cos_lat1 = cos(sp1);
	es_sin1 = _es * sin_lat1;
	one_minus_sqr_es_sin1 = ONE_MINUS_SQR(es_sin1);
	m1 = ALBERS_M(cos_lat1, one_minus_sqr_es_sin1);
	q1 = ALBERS_Q(sin_lat1, one_minus_sqr_es_sin1, es_sin1);

	sqr_m1 = m1 * m1;
	if (fabs(sp1 - sp2) > 1.0e-10) {
		sin_lat2 = sin(sp2);
		cos_lat2 = cos(sp2);
		es_sin2 = _es * sin_lat2;
		one_minus_sqr_es_sin2 = ONE_MINUS_SQR(es_sin2);
		m2 = ALBERS_M(cos_lat2, one_minus_sqr_es_sin2);
		q2 = ALBERS_Q(sin_lat2, one_minus_sqr_es_sin2, es_sin2);
		sqr_m2 = m2 * m2;
		_n = (sqr_m1 - sqr_m2) / (q2 - q1);
	} else
		_n = sin_lat1;

	_C = sqr_m1 + _n * q1;
	_a_over_n = ellipsoid->radius() / _n;
	nq0 = _n * q0;
	_rho0 = (_C < nq0) ? 0 : _a_over_n * sqrt(_C - nq0);
}

QPointF AlbersEqual::ll2xy(const Coordinates &c) const
{
	double dlam;
	double sin_lat;
	double es_sin;
	double q;
	double rho;
	double theta;
	double nq;


	dlam = deg2rad(c.lon()) - _longitudeOrigin;
	if (dlam > M_PI)
		dlam -= 2.0 * M_PI;
	if (dlam < -M_PI)
		dlam += 2.0 * M_PI;

	sin_lat = sin(deg2rad(c.lat()));
	es_sin = _es * sin_lat;
	q = ALBERS_Q(sin_lat, ONE_MINUS_SQR(es_sin), es_sin);
	nq = _n * q;
	rho = (_C < nq) ? 0 : _a_over_n * sqrt(_C - nq);
	theta = _n * dlam;

	return QPointF(rho * sin(theta) + _falseEasting,
	  _rho0 - rho * cos(theta) + _falseNorthing);
}

Coordinates AlbersEqual::xy2ll(const QPointF &p) const
{
	double dy, dx;
	double rho0_minus_dy;
	double q, qc, q_over_2;
	double rho, rho_n;
	double phi, delta_phi = 1.0;
	double sin_phi;
	double es_sin, one_minus_sqr_es_sin;
	double theta = 0.0;
	int count = 30;
	double tolerance = 4.85e-10;
	double lat, lon;


	dy = p.y() - _falseNorthing;
	dx = p.x() - _falseEasting;

	rho0_minus_dy = _rho0 - dy;
	rho = sqrt(dx * dx + rho0_minus_dy * rho0_minus_dy);

	if (_n < 0) {
		rho *= -1.0;
		dx *= -1.0;
		rho0_minus_dy *= -1.0;
	}

	if (rho != 0.0)
		theta = atan2(dx, rho0_minus_dy);
	rho_n = rho * _n;
	q = (_C - (rho_n * rho_n) / _a2) / _n;
	qc = 1 - ((_one_minus_es2) / (_two_es)) * log((1.0 - _es) / (1.0 + _es));
	if (fabs(fabs(qc) - fabs(q)) > 1.0e-6) {
		q_over_2 = q / 2.0;
		if (q_over_2 > 1.0)
			lat = M_PI_2;
		else if (q_over_2 < -1.0)
			lat = -M_PI_2;
		else {
			phi = asin(q_over_2);
			if (_es < 1.0e-10)
				lat = phi;
			else  {
				while ((fabs(delta_phi) > tolerance) && count) {
					sin_phi = sin(phi);
					es_sin = _es * sin_phi;
					one_minus_sqr_es_sin = ONE_MINUS_SQR(es_sin);
					delta_phi = (one_minus_sqr_es_sin * one_minus_sqr_es_sin)
					  / (2.0 * cos(phi)) * (q / (_one_minus_es2) - sin_phi
					  / one_minus_sqr_es_sin + (log((1.0 - es_sin)
					  / (1.0 + es_sin)) / (_two_es)));
					phi += delta_phi;
					count --;
				}

				lat = phi;
			}

			if (lat > M_PI_2)
				lat = M_PI_2;
			else if (lat < -M_PI_2)
				lat = -M_PI_2;
		}
	} else {
		if (q >= 0.0)
			lat = M_PI_2;
		else
			lat = -M_PI_2;
	}

	lon = _longitudeOrigin + theta / _n;

	if (lon > M_PI)
		lon -= M_PI * 2;
	if (lon < -M_PI)
		lon += M_PI * 2;

	if (lon > M_PI)
		lon = M_PI;
	else if (lon < -M_PI)
		lon = -M_PI;

	return Coordinates(rad2deg(lon), rad2deg(lat));
}
