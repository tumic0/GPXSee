#include <cmath>
#include "rd.h"
#include "wgs84.h"
#include "transversemercator.h"

TransverseMercator::TransverseMercator(double centralMeridian, double scale,
  double falseEasting, double falseNorthing)
{
	_centralMeridian = centralMeridian;
	_scale = scale;
	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;
}

QPointF TransverseMercator::ll2xy(const Coordinates &c) const
{
	QPointF p;

	const double e2 = WGS84_FLATTENING * (2 - WGS84_FLATTENING);
	const double n = WGS84_FLATTENING / (2 - WGS84_FLATTENING);
	const double rectifyingRadius = WGS84_RADIUS / (1 + n)
	  * (1 + 0.25*pow(n, 2) + 0.015625*pow(n, 4));

	double A = e2;
	double B = (5 * pow(e2, 2) - pow(e2, 3)) / 6.0;
	double C = (104 * pow(e2, 3) - 45 * pow(e2, 4)) / 120.0;
	double D = (1237 * pow(e2, 4)) / 1260.0;

	double phi = deg2rad(c.lat());
	double lambda = deg2rad(c.lon());
	double lambda0 = deg2rad(_centralMeridian);

	double deltaLambda = lambda - lambda0;

	double phiStar = phi - sin(phi) * cos(phi) * (A + B*pow(sin(phi), 2)
	  + C*pow(sin(phi), 4) + D*pow(sin(phi), 6));

	double xiPrim = atan(tan(phiStar) / cos(deltaLambda));
	double etaPrim = atanh(cos(phiStar) * sin(deltaLambda));

	double beta1 = 1/2.0 * n - 2/3.0 * pow(n, 2) + 5/16.0 * pow(n, 3)
	  + 41/180.0 * pow(n, 4);
	double beta2 = 13/48.0 * pow(n, 2) - 3/5.0 * pow(n, 3) + 557/1440.0
	  * pow(n, 4);
	double beta3 = 61/240.0 * pow(n, 3) - 103/140.0 * pow(n, 4);
	double beta4 = 49561/161280.0 * pow(n, 4);

	p.ry() = _falseNorthing + _scale * rectifyingRadius * (xiPrim + beta1
	  * sin(2*xiPrim) * cosh(2*etaPrim) + beta2 * sin(4*xiPrim)
	  * cosh(4*etaPrim) + beta3 * sin(6*xiPrim) * cosh(6*etaPrim) + beta4
	  * sin(8*xiPrim) * cosh(8*etaPrim));
	p.rx() = _falseEasting + _scale * rectifyingRadius * (etaPrim + beta1
	  * cos(2*xiPrim) * sinh(2*etaPrim) + beta2 * cos(4*xiPrim)
	  * sinh(4*etaPrim) + beta3 * cos(6*xiPrim) * sinh(6*etaPrim) + beta4
	  * cos(8*xiPrim) * sinh(8*etaPrim));

	return p;
}

Coordinates TransverseMercator::xy2ll(const QPointF &p) const
{
	const double e2 = WGS84_FLATTENING * (2 - WGS84_FLATTENING);
	const double n = WGS84_FLATTENING / (2 - WGS84_FLATTENING);
	const double rectifyingRadius = WGS84_RADIUS / (1 + n)
	  * (1 + 0.25*pow(n, 2) + 0.015625*pow(n, 4));

	double xi = (p.y() - _falseNorthing) / (_scale * rectifyingRadius);
	double eta = (p.x() - _falseEasting) / (_scale * rectifyingRadius);

	double delta1 = 1/2.0 * n - 2/3.0 * pow(n, 2) + 37/96.0 * pow(n, 3)
	  - 1/360.0 * pow(n, 4);
	double delta2 = 1/48.0 * pow(n, 2) + 1/15.0 * pow(n, 3) - 437/1440.0
	  * pow(n, 4);
	double delta3 = 17/480.0 * pow(n, 3) - 37/840.0 * pow(n, 4);
	double delta4 = 4397/161280.0 * pow(n, 4);

	double xiPrim = xi - delta1 * sin(2*xi) * cosh(2*eta) - delta2 * sin(4*xi)
	  * cosh(4*eta) - delta3 * sin(6*xi) * cosh(6*eta) - delta4 * sin(8*xi)
	  * cosh(8*eta);
	double etaPrim = eta - delta1 * cos(2*xi) * sinh(2*eta) - delta2 * cos(4*xi)
	  * sinh(4*eta) - delta3 * cos(6*xi) * sinh(6*eta) - delta4 * cos(8*xi)
	  * sinh(8*eta);

	double phiStar = asin(sin(xiPrim) / cosh(etaPrim));
	double deltaLambda = atan(sinh(etaPrim) / cos(xiPrim));

	double AStar = e2 + pow(e2, 2) + pow(e2, 3) + pow(e2, 4);
	double BStar = (7 * pow(e2, 2) + 17 * pow(e2, 3) + 30 * pow(e2, 4)) / -6;
	double CStar = (224 * pow(e2, 3) + 889 * pow(e2, 4)) / 120;
	double DStar = (4279 * pow(e2, 4)) / -1260;

	double phi = phiStar + sin(phiStar) * cos(phiStar) * (AStar + BStar
	  * pow(sin(phiStar), 2) + CStar * pow(sin(phiStar), 4) + DStar
	  * pow(sin(phiStar), 6));

	return Coordinates(_centralMeridian + rad2deg(deltaLambda), rad2deg(phi));
}
