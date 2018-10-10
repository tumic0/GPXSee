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
#include "mercator.h"

Mercator::Mercator(const Ellipsoid *ellipsoid, double latitudeOrigin,
  double longitudeOrigin, double falseEasting, double falseNorthing)
{
	double es = ellipsoid->es();
	double es2;
	double es3;
	double es4;
	double sin_olat;

	_latitudeOrigin = deg2rad(latitudeOrigin);
	_longitudeOrigin = deg2rad(longitudeOrigin);
	if (_longitudeOrigin > M_PI)
		_longitudeOrigin -= M_2_PI;
	_falseNorthing = falseNorthing;
	_falseEasting = falseEasting;

	_a = ellipsoid->radius();
	_e = sqrt(es);

	sin_olat = sin(_latitudeOrigin);
	_scaleFactor = 1.0 / (sqrt(1.e0 - es * sin_olat * sin_olat)
	  / cos(_latitudeOrigin));
	es2 = es * es;
	es3 = es2 * es;
	es4 = es3 * es;
	_ab = es / 2.e0 + 5.e0 * es2 / 24.e0 + es3 / 12.e0 + 13.e0 * es4 / 360.e0;
	_bb = 7.e0 * es2 / 48.e0 + 29.e0 * es3 / 240.e0 + 811.e0 * es4 / 11520.e0;
	_cb = 7.e0 * es3 / 120.e0 + 81.e0 * es4 / 1120.e0;
	_db = 4279.e0 * es4 / 161280.e0;
}

PointD Mercator::ll2xy(const Coordinates &c) const
{
	double lon = deg2rad(c.lon());
	double lat = deg2rad(c.lat());
	double ctanz2;
	double e_x_sinlat;
	double delta_lon;
	double tan_temp;
	double pow_temp;

	if (lon > M_PI)
		lon -= M_2_PI;
	e_x_sinlat = _e * sin(lat);
	tan_temp = tan(M_PI_4 + lat / 2.e0);
	pow_temp = pow((1.e0 - e_x_sinlat) / (1.e0 + e_x_sinlat), _e / 2.e0);
	ctanz2 = tan_temp * pow_temp;
	delta_lon = lon - _longitudeOrigin;
	if (delta_lon > M_PI)
	  delta_lon -= M_2_PI;
	if (delta_lon < -M_PI)
	  delta_lon += M_2_PI;

	return PointD(_scaleFactor * _a * delta_lon + _falseEasting,
	  _scaleFactor * _a * log(ctanz2) + _falseNorthing);
}

Coordinates Mercator::xy2ll(const PointD &p) const
{
	double dx;
	double dy;
	double xphi;
	double lat, lon;

	dy = p.y() - _falseNorthing;
	dx = p.x() - _falseEasting;
	lon = _longitudeOrigin + dx / (_scaleFactor * _a);
	xphi = M_PI_2 - 2.e0 * atan(1.e0 / exp(dy / (_scaleFactor * _a)));
	lat = xphi + _ab * sin(2.e0 * xphi) + _bb * sin(4.e0 * xphi)
	  + _cb * sin(6.e0 * xphi) + _db * sin(8.e0 * xphi);
	if (lon > M_PI)
		lon -= M_2_PI;
	if (lon < -M_PI)
		lon += M_2_PI;

	return Coordinates(rad2deg(lon), rad2deg(lat));
}
