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
#include "map/ellipsoid.h"
#include "transversemercator.h"

#define SPHSN(lat) \
	((double)(_a / sqrt(1.e0 - _es * pow(sin(lat), 2))))
#define SPHTMD(lat) \
	((double)(_ap * lat - _bp * sin(2.e0 * lat) + _cp * sin(4.e0 * lat) \
	  - _dp * sin(6.e0 * lat) + _ep * sin(8.e0 * lat)))
#define DENOM(lat) \
	((double)(sqrt(1.e0 - _es * pow(sin(lat),2))))
#define SPHSR(lat) \
	((double)(_a * (1.e0 - _es) / pow(DENOM(lat), 3)))

TransverseMercator::TransverseMercator(const Ellipsoid &ellipsoid,
  double latitudeOrigin, double longitudeOrigin, double scale,
  double falseEasting, double falseNorthing)
{
	_a = ellipsoid.radius();
	_longitudeOrigin = deg2rad(longitudeOrigin);
	_latitudeOrigin = deg2rad(latitudeOrigin);
	_scale = scale;
	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;

	_es = ellipsoid.es();
	_ebs = (1 / (1 - _es)) - 1;
	double b = ellipsoid.b();

	double tn = (_a - b) / (_a + b);
	double tn2 = tn * tn;
	double tn3 = tn2 * tn;
	double tn4 = tn3 * tn;
	double tn5 = tn4 * tn;

	_ap = _a * (1.e0 - tn + 5.e0 * (tn2 - tn3) / 4.e0 + 81.e0
	  * (tn4 - tn5) / 64.e0);
	_bp = 3.e0 * _a * (tn - tn2 + 7.e0 * (tn3 - tn4) / 8.e0 + 55.e0
	  * tn5 / 64.e0 ) / 2.e0;
	_cp = 15.e0 * _a * (tn2 - tn3 + 3.e0 * (tn4 - tn5 ) / 4.e0) / 16.0;
	_dp = 35.e0 * _a * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
	_ep = 315.e0 * _a * (tn4 - tn5) / 512.e0;
}

PointD TransverseMercator::ll2xy(const Coordinates &c) const
{
	double dlam = deg2rad(c.lon()) - _longitudeOrigin;
	if (dlam > M_PI)
		dlam -= 2 * M_PI;
	if (dlam < -M_PI)
		dlam += 2 * M_PI;
	if (fabs(dlam) < 2.e-10)
		dlam = 0.0;

	double rl = deg2rad(c.lat());
	double sl = sin(rl);
	double cl = cos(rl);
	double c2 = cl * cl;
	double c3 = c2 * cl;
	double c5 = c3 * c2;
	double c7 = c5 * c2;
	double t = sl / cl;
	double tan2 = t * t;
	double tan3 = tan2 * t;
	double tan4 = tan3 * t;
	double tan5 = tan4 * t;
	double tan6 = tan5 * t;
	double eta = _ebs * c2;
	double eta2 = eta * eta;
	double eta3 = eta2 * eta;
	double eta4 = eta3 * eta;

	double sn = SPHSN(rl);
	double tmd = SPHTMD(rl);
	double tmdo = SPHTMD (_latitudeOrigin);

	double t1 = (tmd - tmdo) * _scale;
	double t2 = sn * sl * cl * _scale / 2.e0;
	double t3 = sn * sl * c3 * _scale * (5.e0 - tan2 + 9.e0 * eta + 4.e0 * eta2)
	  / 24.e0;
	double t4 = sn * sl * c5 * _scale * (61.e0 - 58.e0 * tan2 + tan4 + 270.e0
	  * eta - 330.e0 * tan2 * eta + 445.e0 * eta2 + 324.e0 * eta3 - 680.e0
	  * tan2 * eta2 + 88.e0 * eta4 - 600.e0 * tan2 * eta3 - 192.e0 * tan2
	  * eta4) / 720.e0;
	double t5 = sn * sl * c7 * _scale * (1385.e0 - 3111.e0 * tan2 + 543.e0
	  * tan4 - tan6) / 40320.e0;

	double y = _falseNorthing + t1 + pow(dlam, 2.e0) * t2 + pow(dlam, 4.e0) * t3
	  + pow(dlam, 6.e0) * t4 + pow(dlam, 8.e0) * t5;

	double t6 = sn * cl * _scale;
	double t7 = sn * c3 * _scale * (1.e0 - tan2 + eta) /6.e0;
	double t8 = sn * c5 * _scale * (5.e0 - 18.e0 * tan2 + tan4 + 14.e0 * eta
	  - 58.e0 * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 - 64.e0 * tan2 * eta2
	  - 24.e0 * tan2 * eta3) / 120.e0;
	double t9 = sn * c7 * _scale * (61.e0 - 479.e0 * tan2 + 179.e0 * tan4
	  - tan6) / 5040.e0;

	double x = _falseEasting + dlam * t6 + pow(dlam, 3.e0) * t7
	  + pow(dlam, 5.e0) * t8 + pow(dlam, 7.e0) * t9;

	return PointD(x, y);
}

Coordinates TransverseMercator::xy2ll(const PointD &p) const
{
	double t10;

	double tmdo = SPHTMD(_latitudeOrigin);
	double tmd = tmdo + (p.y() - _falseNorthing) / _scale;

	double sr = SPHSR(0.e0);
	double ftphi = tmd / sr;

	for (int i = 0; i < 5 ; i++) {
		t10 = SPHTMD(ftphi);
		sr = SPHSR(ftphi);
		ftphi = ftphi + (tmd - t10) / sr;
	}

	sr = SPHSR(ftphi);
	double sn = SPHSN(ftphi);

	double cl = cos(ftphi);

	double t = tan(ftphi);
	double tan2 = t * t;
	double tan4 = tan2 * tan2;
	double eta = _ebs * pow(cl, 2);
	double eta2 = eta * eta;
	double eta3 = eta2 * eta;
	double eta4 = eta3 * eta;
	double de = p.x() - _falseEasting;
	if (fabs(de) < 0.0001)
		de = 0.0;

	t10 = t / (2.e0 * sr * sn * pow(_scale, 2));
	double t11 = t * (5.e0  + 3.e0 * tan2 + eta - 4.e0 * pow(eta, 2) - 9.e0
	  * tan2 * eta) / (24.e0 * sr * pow(sn, 3) * pow(_scale, 4));
	double t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4 - 252.e0
	  * tan2 * eta - 3.e0 * eta2 + 100.e0 * eta3 - 66.e0 * tan2 * eta2 - 90.e0
	  * tan4 * eta + 88.e0 * eta4 + 225.e0 * tan4 * eta2 + 84.e0 * tan2 * eta3
	  - 192.e0 * tan2 * eta4) / (720.e0 * sr * pow(sn, 5) * pow(_scale, 6));
	double t13 = t * (1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0
	  * pow(t,6)) / (40320.e0 * sr * pow(sn, 7) * pow(_scale, 8));
	double lat = ftphi - pow(de, 2) * t10 + pow(de, 4) * t11 - pow(de, 6) * t12
	  + pow(de, 8) * t13;

	double t14 = 1.e0 / (sn * cl * _scale);
	double t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn, 3) * cl
	  * pow(_scale, 3));
	double t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2 + 8.e0 * tan2
	  * eta + 24.e0 * tan4 - 4.e0 * eta3 + 4.e0 * tan2 * eta2 + 24.e0 * tan2
	  * eta3) / (120.e0 * pow(sn, 5) * cl * pow(_scale, 5));
	double t17 = (61.e0 +  662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 * pow(t,6))
	  / (5040.e0 * pow(sn, 7) * cl * pow(_scale, 7));

	double dlam = de * t14 - pow(de, 3) * t15 + pow(de, 5) * t16 - pow(de, 7)
	  * t17;

	double lon = _longitudeOrigin + dlam;
	while (lat > deg2rad(90.0)) {
		lat = M_PI - lat;
		lon += M_PI;
		if (lon > M_PI)
			lon -= 2 * M_PI;
	}

	while (lat < deg2rad(-90.0)) {
		lat = - (lat + M_PI);
		lon += M_PI;
		if (lon > M_PI)
			lon -= 2 * M_PI;
	}

	if (lon > 2 * M_PI)
		lon -= 2 * M_PI;
	if (lon < -M_PI)
		lon += 2 * M_PI;

	return Coordinates(rad2deg(lon), rad2deg(lat));
}

bool TransverseMercator::operator==(const CT &ct) const
{
	const TransverseMercator *other
	  = dynamic_cast<const TransverseMercator*>(&ct);
	return (other != 0 && _longitudeOrigin == other->_longitudeOrigin
	  && _latitudeOrigin == other->_latitudeOrigin && _scale == other->_scale
	  && _falseEasting == other->_falseEasting
	  && _falseNorthing == other->_falseNorthing && _a == other->_a
	  && _es == other->_es);
}
