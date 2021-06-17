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
#include "polarstereographic.h"


#define POLAR_POW(EsSin) pow((1.0 - EsSin) / (1.0 + EsSin), _es_OVER_2)

PolarStereographic::PolarStereographic(const Ellipsoid &ellipsoid,
  double latitudeOrigin, double longitudeOrigin,
  double falseEasting, double falseNorthing)
{
	_two_a = 2.0 * ellipsoid.radius();

	if (longitudeOrigin > M_PI)
		longitudeOrigin -= 2*M_PI;

	if (latitudeOrigin < 0) {
		_southernHemisphere = 1;
		_originLatitude = -deg2rad(latitudeOrigin);
		_originLongitude = -deg2rad(longitudeOrigin);
	} else {
		_southernHemisphere = 0;
		_originLatitude = deg2rad(latitudeOrigin);
		_originLongitude = deg2rad(longitudeOrigin);
	}
	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;

	double es2 = ellipsoid.es();
	_es = sqrt(es2);
	_es_OVER_2 = _es / 2.0;

	if (fabs(fabs(_originLatitude) - M_PI_2) > 1.0e-10) {
		double slat = sin(_originLatitude);
		double essin = _es * slat;
		double pow_es = POLAR_POW(essin);
		double clat = cos(_originLatitude);
		_mc = clat / sqrt(1.0 - essin * essin);
		_a_mc = ellipsoid.radius() * _mc;
		_tc = tan(M_PI_4 - _originLatitude / 2.0) / pow_es;
	} else {
		double one_PLUS_es = 1.0 + _es;
		double one_MINUS_es = 1.0 - _es;
		_e4 = sqrt(pow(one_PLUS_es, one_PLUS_es) * pow(one_MINUS_es,
		  one_MINUS_es));
	}
}

PointD PolarStereographic::ll2xy(const Coordinates &c) const
{
	double Easting, Northing;
	double Longitude = deg2rad(c.lon());
	double Latitude = deg2rad(c.lat());


	if (fabs(fabs(Latitude) - M_PI_2) < 1.0e-10) {
		Easting = 0.0;
		Northing = 0.0;
	} else {
		if (_southernHemisphere != 0) {
			Longitude *= -1.0;
			Latitude *= -1.0;
		}

		double dlam = Longitude - _originLongitude;
		if (dlam > M_PI)
			dlam -= 2*M_PI;
		if (dlam < -M_PI)
			dlam += 2*M_PI;

		double slat = sin(Latitude);
		double essin = _es * slat;
		double pow_es = POLAR_POW(essin);
		double t = tan(M_PI_4 - Latitude / 2.0) / pow_es;

		double rho;
		if (fabs(fabs(_originLatitude) - M_PI_2) > 1.0e-10)
			rho = _a_mc * t / _tc;
		else
			rho = _two_a * t / _e4;

		Easting = rho * sin(dlam) + _falseEasting;

		if (_southernHemisphere != 0) {
			Easting *= -1.0;
			Northing = rho * cos(dlam) + _falseNorthing;
		} else
			Northing = -rho * cos(dlam) + _falseNorthing;
	}

	return PointD(Easting, Northing);
}

Coordinates PolarStereographic::xy2ll(const PointD &p) const
{
	double Latitude, Longitude;
	double dy = p.y() - _falseNorthing;
	double dx = p.x() - _falseEasting;

	if ((dy == 0.0) && (dx == 0.0)) {
		Latitude = M_PI_2;
		Longitude = _originLongitude;
	} else {
		if (_southernHemisphere != 0) {
			dy *= -1.0;
			dx *= -1.0;
		}

		double rho = sqrt(dx * dx + dy * dy);
		double t;
		if (fabs(fabs(_originLatitude) - M_PI_2) > 1.0e-10)
			t = rho * _tc / (_a_mc);
		else
			t = rho * _e4 / (_two_a);
		double PHI = M_PI_2 - 2.0 * atan(t);

		double tempPHI = 0.0;
		while (fabs(PHI - tempPHI) > 1.0e-10) {
			tempPHI = PHI;
			double sin_PHI = sin(PHI);
			double essin =  _es * sin_PHI;
			double pow_es = POLAR_POW(essin);
			PHI = M_PI_2 - 2.0 * atan(t * pow_es);
		}
		Latitude = PHI;
		Longitude = _originLongitude + atan2(dx, -dy);

		if (Longitude > M_PI)
			Longitude -= 2*M_PI;
		else if (Longitude < -M_PI)
			Longitude += 2*M_PI;

		if (Latitude > M_PI_2)
			Latitude = M_PI_2;
		else if (Latitude < -M_PI_2)
			Latitude = -M_PI_2;

		if (Longitude > M_PI)
			Longitude = M_PI;
		else if (Longitude < -M_PI)
			Longitude = -M_PI;
	}

	if (_southernHemisphere != 0) {
		Latitude *= -1.0;
		Longitude *= -1.0;
	}

	return Coordinates(rad2deg(Longitude), rad2deg(Latitude));
}

bool PolarStereographic::operator==(const CT &ct) const
{
	const PolarStereographic *other
	  = dynamic_cast<const PolarStereographic*>(&ct);
	return (other != 0 && _originLatitude == other->_originLatitude
	  && _originLongitude == other->_originLongitude
	  && _falseEasting == other->_falseEasting
	  && _falseNorthing == other->_falseNorthing && _two_a == other->_two_a
	  && _es == other->_es && _southernHemisphere == other->_southernHemisphere);
}
