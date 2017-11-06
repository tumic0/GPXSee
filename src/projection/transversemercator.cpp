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
#include "rd.h"
#include "ellipsoid.h"
#include "transversemercator.h"


#define SPHSN(lat) \
	((double)(_e.radius() / sqrt(1.e0 - _es * pow(sin(lat), 2))))
#define SPHTMD(lat) \
	((double)(_ap * lat - _bp * sin(2.e0 * lat) + _cp * sin(4.e0 * lat) \
	  - _dp * sin(6.e0 * lat) + _ep * sin(8.e0 * lat)))
#define DENOM(lat) \
	((double)(sqrt(1.e0 - _es * pow(sin(lat),2))))
#define SPHSR(lat) \
	((double)(_e.radius() * (1.e0 - _es) / pow(DENOM(lat), 3)))


TransverseMercator::TransverseMercator(const Ellipsoid &ellipsoid,
  double latitudeOrigin, double longitudeOrigin, double scale,
  double falseEasting, double falseNorthing)
{
	double tn, tn2, tn3, tn4, tn5;
	double b;


	_e = ellipsoid;
	_longitudeOrigin = deg2rad(longitudeOrigin);
	_latitudeOrigin = deg2rad(latitudeOrigin);
	_scale = scale;
	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;

	_es = 2 * _e.flattening() - _e.flattening() * _e.flattening();
	_ebs = (1 / (1 - _es)) - 1;

	b = _e.radius() * (1 - _e.flattening());

	tn = (_e.radius() - b) / (_e.radius() + b);
	tn2 = tn * tn;
	tn3 = tn2 * tn;
	tn4 = tn3 * tn;
	tn5 = tn4 * tn;

	_ap = _e.radius() * (1.e0 - tn + 5.e0 * (tn2 - tn3) / 4.e0 + 81.e0
	  * (tn4 - tn5) / 64.e0);
	_bp = 3.e0 * _e.radius() * (tn - tn2 + 7.e0 * (tn3 - tn4) / 8.e0 + 55.e0
	  * tn5 / 64.e0 ) / 2.e0;
	_cp = 15.e0 * _e.radius() * (tn2 - tn3 + 3.e0 * (tn4 - tn5 ) / 4.e0) / 16.0;
	_dp = 35.e0 * _e.radius() * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
	_ep = 315.e0 * _e.radius() * (tn4 - tn5) / 512.e0;
}

QPointF TransverseMercator::ll2xy(const Coordinates &c) const
{
	double rl;
	double cl, c2, c3, c5, c7;
	double dlam;
	double eta, eta2, eta3, eta4;
	double sl, sn;
	double t, tan2, tan3, tan4, tan5, tan6;
	double t1, t2, t3, t4, t5, t6, t7, t8, t9;
	double tmd, tmdo;
	double x, y;


	dlam = deg2rad(c.lon()) - _longitudeOrigin;

	if (dlam > M_PI)
		dlam -= (2 * M_PI);
	if (dlam < -M_PI)
		dlam += (2 * M_PI);
	if (fabs(dlam) < 2.e-10)
		dlam = 0.0;

	rl = deg2rad(c.lat());
	sl = sin(rl);
	cl = cos(rl);
	c2 = cl * cl;
	c3 = c2 * cl;
	c5 = c3 * c2;
	c7 = c5 * c2;
	t = sl / cl;
	tan2 = t * t;
	tan3 = tan2 * t;
	tan4 = tan3 * t;
	tan5 = tan4 * t;
	tan6 = tan5 * t;
	eta = _ebs * c2;
	eta2 = eta * eta;
	eta3 = eta2 * eta;
	eta4 = eta3 * eta;

	sn = SPHSN(rl);
	tmd = SPHTMD(rl);
	tmdo = SPHTMD (_latitudeOrigin);


	t1 = (tmd - tmdo) * _scale;
	t2 = sn * sl * cl * _scale / 2.e0;
	t3 = sn * sl * c3 * _scale * (5.e0 - tan2 + 9.e0 * eta + 4.e0 * eta2)
	  / 24.e0;
	t4 = sn * sl * c5 * _scale * (61.e0 - 58.e0 * tan2 + tan4 + 270.e0 * eta
	  - 330.e0 * tan2 * eta + 445.e0 * eta2 + 324.e0 * eta3 - 680.e0 * tan2
	  * eta2 + 88.e0 * eta4 - 600.e0 * tan2 * eta3 - 192.e0 * tan2 * eta4)
	  / 720.e0;
	t5 = sn * sl * c7 * _scale * (1385.e0 - 3111.e0 * tan2 + 543.e0 * tan4
	  - tan6) / 40320.e0;

	y = _falseNorthing + t1 + pow(dlam, 2.e0) * t2 + pow(dlam, 4.e0) * t3
	  + pow(dlam, 6.e0) * t4 + pow(dlam, 8.e0) * t5;


	t6 = sn * cl * _scale;
	t7 = sn * c3 * _scale * (1.e0 - tan2 + eta) /6.e0;
	t8 = sn * c5 * _scale * (5.e0 - 18.e0 * tan2 + tan4 + 14.e0 * eta - 58.e0
	  * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 - 64.e0 * tan2 * eta2 - 24.e0
	  * tan2 * eta3) / 120.e0;
	t9 = sn * c7 * _scale * (61.e0 - 479.e0 * tan2 + 179.e0 * tan4 - tan6)
	  / 5040.e0;

	x = _falseEasting + dlam * t6 + pow(dlam, 3.e0) * t7 + pow(dlam, 5.e0)
	  * t8 + pow(dlam, 7.e0) * t9;

	return QPointF(x, y);
}

Coordinates TransverseMercator::xy2ll(const QPointF &p) const
{
	double cl;
	double de;
	double dlam;
	double eta, eta2, eta3, eta4;
	double ftphi;
	double sn;
	double sr;
	double t, tan2, tan4;
	double t10, t11, t12, t13, t14, t15, t16, t17;
	double tmd, tmdo;
	double lat, lon;


	tmdo = SPHTMD(_latitudeOrigin);
	tmd = tmdo + (p.y() - _falseNorthing) / _scale;

	sr = SPHSR(0.e0);
	ftphi = tmd / sr;

	for (int i = 0; i < 5 ; i++) {
		t10 = SPHTMD(ftphi);
		sr = SPHSR(ftphi);
		ftphi = ftphi + (tmd - t10) / sr;
	}

	sr = SPHSR(ftphi);
	sn = SPHSN(ftphi);

	cl = cos(ftphi);

	t = tan(ftphi);
	tan2 = t * t;
	tan4 = tan2 * tan2;
	eta = _ebs * pow(cl, 2);
	eta2 = eta * eta;
	eta3 = eta2 * eta;
	eta4 = eta3 * eta;
	de = p.x() - _falseEasting;
	if (fabs(de) < 0.0001)
		de = 0.0;

	t10 = t / (2.e0 * sr * sn * pow(_scale, 2));
	t11 = t * (5.e0  + 3.e0 * tan2 + eta - 4.e0 * pow(eta, 2) - 9.e0 * tan2
	  * eta) / (24.e0 * sr * pow(sn, 3) * pow(_scale, 4));
	t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4 - 252.e0 * tan2
	  * eta - 3.e0 * eta2 + 100.e0 * eta3 - 66.e0 * tan2 * eta2 - 90.e0 * tan4
	  * eta + 88.e0 * eta4 + 225.e0 * tan4 * eta2 + 84.e0 * tan2 * eta3 - 192.e0
	  * tan2 * eta4) / (720.e0 * sr * pow(sn, 5) * pow(_scale, 6));
	t13 = t * (1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0 * pow(t,6))
	  / (40320.e0 * sr * pow(sn, 7) * pow(_scale, 8));
	lat = ftphi - pow(de, 2) * t10 + pow(de, 4) * t11 - pow(de, 6) * t12
	  + pow(de, 8) * t13;

	t14 = 1.e0 / (sn * cl * _scale);
	t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn, 3) * cl * pow(_scale, 3));
	t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2 + 8.e0 * tan2 * eta
	  + 24.e0 * tan4 - 4.e0 * eta3 + 4.e0 * tan2 * eta2 + 24.e0 * tan2 * eta3)
	  / (120.e0 * pow(sn, 5) * cl * pow(_scale, 5));
	t17 = (61.e0 +  662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 * pow(t,6))
	  / (5040.e0 * pow(sn, 7) * cl * pow(_scale, 7));

	dlam = de * t14 - pow(de, 3) * t15 + pow(de, 5) * t16 - pow(de, 7) * t17;

	lon = _longitudeOrigin + dlam;
	while (lat > deg2rad(90.0)) {
		lat = M_PI - lat;
		lon += M_PI;
		if (lon > M_PI)
			lon -= (2 * M_PI);
	}

	while (lat < deg2rad(-90.0)) {
		lat = - (lat + M_PI);
		lon += M_PI;
		if (lon > M_PI)
			lon -= (2 * M_PI);
	}

	if (lon > (2 * M_PI))
		lon -= (2 * M_PI);
	if (lon < -M_PI)
		lon += (2 * M_PI);

	return Coordinates(rad2deg(lon), rad2deg(lat));
}
