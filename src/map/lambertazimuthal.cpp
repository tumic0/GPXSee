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

#include "lambertazimuthal.h"


#ifndef M_PI_2
	#define M_PI_2 1.57079632679489661923
#endif // M_PI_2

LambertAzimuthal::LambertAzimuthal(const Ellipsoid *ellipsoid,
  double latitudeOrigin, double longitudeOrigin, double falseEasting,
  double falseNorthing)
{
	double es2, es4, es6;

	es2 = 2 * ellipsoid->flattening() - ellipsoid->flattening()
	  * ellipsoid->flattening();

	es4 = es2 * es2;
	es6 = es4 * es2;
	_ra = ellipsoid->radius() * (1.0 - es2 / 6.0 - 17.0 * es4 / 360.0 - 67.0
	  * es6 / 3024.0);
	_latOrigin = deg2rad(latitudeOrigin);
	_sinLatOrigin = sin(_latOrigin);
	_cosLatOrigin = cos(_latOrigin);
	_absLatOrigin = fabs(_latOrigin);
	_lonOrigin = deg2rad(longitudeOrigin);
	if (_lonOrigin > M_PI)
		_lonOrigin -= 2.0 * M_PI;

	_falseNorthing = falseNorthing;
	_falseEasting = falseEasting;
}

QPointF LambertAzimuthal::ll2xy(const Coordinates &c) const
{
	double dlam;
	double k_prime;
	double cd;
	double rlat = deg2rad(c.lat());
	double slat = sin(rlat);
	double clat = cos(rlat);
	double cos_c;
	double sin_dlam, cos_dlam;
	double Ra_kprime;
	double Ra_PI_OVER_2_Lat;
	QPointF p;


	dlam = deg2rad(c.lon()) - _lonOrigin;
	if (dlam > M_PI)
		dlam -= 2.0 * M_PI;
	if (dlam < -M_PI)
		dlam += 2.0 * M_PI;

	sin_dlam = sin(dlam);
	cos_dlam = cos(dlam);
	if (fabs(_absLatOrigin - M_PI_2) < 1.0e-10) {
		if (_latOrigin >= 0.0) {
			Ra_PI_OVER_2_Lat = _ra * (M_PI_2 - rlat);
			p.rx() = Ra_PI_OVER_2_Lat * sin_dlam + _falseEasting;
			p.ry() = -1.0 * (Ra_PI_OVER_2_Lat * cos_dlam) + _falseNorthing;
		} else {
			Ra_PI_OVER_2_Lat = _ra * (M_PI_2 + rlat);
			p.rx() = Ra_PI_OVER_2_Lat * sin_dlam + _falseEasting;
			p.ry() = Ra_PI_OVER_2_Lat * cos_dlam + _falseNorthing;
		}
	} else if (_absLatOrigin <= 1.0e-10) {
		cos_c = clat * cos_dlam;
		if (fabs(fabs(cos_c) - 1.0) < 1.0e-14) {
			if (cos_c >= 0.0) {
				p.rx() = _falseEasting;
				p.ry() = _falseNorthing;
			} else
				return QPointF(NAN, NAN);
		} else {
			cd = acos(cos_c);
			k_prime = cd / sin(cd);
			Ra_kprime = _ra * k_prime;
			p.rx() = Ra_kprime * clat * sin_dlam + _falseEasting;
			p.ry() = Ra_kprime * slat + _falseNorthing;
		}
	} else {
		cos_c = (_sinLatOrigin * slat) + (_cosLatOrigin * clat * cos_dlam);
		if (fabs(fabs(cos_c) - 1.0) < 1.0e-14) {
			if (cos_c >= 0.0) {
				p.rx() = _falseEasting;
				p.ry() = _falseNorthing;
			} else
				return QPointF(NAN, NAN);
		} else {
			cd = acos(cos_c);
			k_prime = cd / sin(cd);
			Ra_kprime = _ra * k_prime;
			p.rx() = Ra_kprime * clat * sin_dlam + _falseEasting;
			p.ry() = Ra_kprime * (_cosLatOrigin * slat - _sinLatOrigin * clat
			  * cos_dlam) + _falseNorthing;
		}
	}

	return p;
}

Coordinates LambertAzimuthal::xy2ll(const QPointF &p) const
{
	double dx, dy;
	double rho;
	double cd;
	double sin_c, cos_c, dy_sinc;
	double lat, lon;


	dy = p.y() - _falseNorthing;
	dx = p.x() - _falseEasting;
	rho = sqrt(dx * dx + dy * dy);
	if (fabs(rho) <= 1.0e-10) {
		lat = _latOrigin;
		lon = _lonOrigin;
	} else {
		cd = rho / _ra;
		sin_c = sin(cd);
		cos_c = cos(cd);
		dy_sinc = dy * sin_c;
		lat = asin((cos_c * _sinLatOrigin) + ((dy_sinc * _cosLatOrigin) / rho));
		if (fabs(_absLatOrigin - M_PI_2) < 1.0e-10) {
			if (_latOrigin >= 0.0)
				lon = _lonOrigin + atan2(dx, -dy);
			else
				lon = _lonOrigin + atan2(dx, dy);
			}
		else
			lon = _lonOrigin + atan2((dx * sin_c), ((rho * _cosLatOrigin
			  * cos_c) - (dy_sinc * _sinLatOrigin)));
	}

	if (lat > M_PI_2)
		lat = M_PI_2;
	else if (lat < -M_PI_2)
		lat = -M_PI_2;

	if (lon > M_PI)
		lon -= 2.0 * M_PI;
	if (lon < -M_PI)
		lon += 2.0 * M_PI;

	if (lon > M_PI)
		lon = M_PI;
	else if (lon < -M_PI)
		lon = -M_PI;

	return Coordinates(rad2deg(lon), rad2deg(lat));
}
