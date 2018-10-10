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
#include "geocentric.h"


#define AD_C 1.0026000

Point3D Geocentric::fromGeodetic(const Coordinates &c, const Ellipsoid *e)
{
	double lat = deg2rad(c.lat());
	double lon = deg2rad(c.lon());
	double slat = sin(lat);
	double slat2 = slat * slat;
	double clat = cos(lat);
	double Rn = e->radius() / (sqrt(1.0 - e->es() * slat2));

	if (lon > M_PI)
		lon -= M_2_PI;

	return Point3D(Rn * clat * cos(lon), Rn * clat * sin(lon),
	  (Rn * (1 - e->es())) * slat);
}

Coordinates Geocentric::toGeodetic(const Point3D &p, const Ellipsoid *e)
{
	bool pole = false;
	double lat, lon;

	if (p.x() == 0.0) {
		if (p.y() > 0)
			lon = M_PI_2;
		else if (p.y() < 0)
			lon = -M_PI_2;
		else {
			pole = true;
			lon = 0.0;
			if (p.z() > 0.0)
				lat = M_PI_2;
			else if (p.z() < 0.0)
				lat = -M_PI_2;
			else
				return Coordinates(rad2deg(lon), rad2deg(M_PI_2));
		}
	} else
		lon = atan2(p.y(), p.x());

	double W2 = p.x()*p.x() + p.y()*p.y();
	double W = sqrt(W2);
	double T0 = p.z() * AD_C;
	double S0 = sqrt(T0 * T0 + W2);
	double Sin_B0 = T0 / S0;
	double Cos_B0 = W / S0;
	double Sin3_B0 = Sin_B0 * Sin_B0 * Sin_B0;
	double T1 = p.z() + e->b() * e->e2s() * Sin3_B0;
	double Sum = W - e->radius() * e->es() * Cos_B0 * Cos_B0 * Cos_B0;
	double S1 = sqrt(T1*T1 + Sum * Sum);
	double Sin_p1 = T1 / S1;
	double Cos_p1 = Sum / S1;

	if (!pole)
		lat = atan(Sin_p1 / Cos_p1);

	return Coordinates(rad2deg(lon), rad2deg(lat));
}
