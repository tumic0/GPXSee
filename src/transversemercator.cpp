#include <cmath>
#include "rd.h"
#include "ellipsoid.h"
#include "transversemercator.h"


TransverseMercator::TransverseMercator(const Ellipsoid &ellipsoid,
  double centralMeridian, double scale, double falseEasting,
  double falseNorthing)
{
	_centralMeridian = centralMeridian;
	_scale = scale;
	_falseEasting = falseEasting;
	_falseNorthing = falseNorthing;


	const double e2 = ellipsoid.flattening() * (2 - ellipsoid.flattening());
	const double n = ellipsoid.flattening() / (2 - ellipsoid.flattening());
	_rectifyingRadius = ellipsoid.radius() / (1 + n)
	  * (1 + 0.25*pow(n, 2) + 0.015625*pow(n, 4));

	_A = e2;
	_B = (5 * pow(e2, 2) - pow(e2, 3)) / 6.0;
	_C = (104 * pow(e2, 3) - 45 * pow(e2, 4)) / 120.0;
	_D = (1237 * pow(e2, 4)) / 1260.0;

	_beta1 = 1/2.0 * n - 2/3.0 * pow(n, 2) + 5/16.0 * pow(n, 3) + 41/180.0
	  * pow(n, 4);
	_beta2 = 13/48.0 * pow(n, 2) - 3/5.0 * pow(n, 3) + 557/1440.0 * pow(n, 4);
	_beta3 = 61/240.0 * pow(n, 3) - 103/140.0 * pow(n, 4);
	_beta4 = 49561/161280.0 * pow(n, 4);

	_delta1 = 1/2.0 * n - 2/3.0 * pow(n, 2) + 37/96.0 * pow(n, 3) - 1/360.0
	  * pow(n, 4);
	_delta2 = 1/48.0 * pow(n, 2) + 1/15.0 * pow(n, 3) - 437/1440.0 * pow(n, 4);
	_delta3 = 17/480.0 * pow(n, 3) - 37/840.0 * pow(n, 4);
	_delta4 = 4397/161280.0 * pow(n, 4);

	_AStar = e2 + pow(e2, 2) + pow(e2, 3) + pow(e2, 4);
	_BStar = (7 * pow(e2, 2) + 17 * pow(e2, 3) + 30 * pow(e2, 4)) / -6;
	_CStar = (224 * pow(e2, 3) + 889 * pow(e2, 4)) / 120;
	_DStar = (4279 * pow(e2, 4)) / -1260;
}

QPointF TransverseMercator::ll2xy(const Coordinates &c) const
{
	QPointF p;

	double phi = deg2rad(c.lat());
	double lambda = deg2rad(c.lon());
	double lambda0 = deg2rad(_centralMeridian);

	double deltaLambda = lambda - lambda0;

	double phiStar = phi - sin(phi) * cos(phi) * (_A + _B*pow(sin(phi), 2)
	  + _C*pow(sin(phi), 4) + _D*pow(sin(phi), 6));

	double xiPrim = atan(tan(phiStar) / cos(deltaLambda));
	double etaPrim = atanh(cos(phiStar) * sin(deltaLambda));

	p.ry() = _falseNorthing + _scale * _rectifyingRadius * (xiPrim + _beta1
	  * sin(2*xiPrim) * cosh(2*etaPrim) + _beta2 * sin(4*xiPrim)
	  * cosh(4*etaPrim) + _beta3 * sin(6*xiPrim) * cosh(6*etaPrim) + _beta4
	  * sin(8*xiPrim) * cosh(8*etaPrim));
	p.rx() = _falseEasting + _scale * _rectifyingRadius * (etaPrim + _beta1
	  * cos(2*xiPrim) * sinh(2*etaPrim) + _beta2 * cos(4*xiPrim)
	  * sinh(4*etaPrim) + _beta3 * cos(6*xiPrim) * sinh(6*etaPrim) + _beta4
	  * cos(8*xiPrim) * sinh(8*etaPrim));

	return p;
}

Coordinates TransverseMercator::xy2ll(const QPointF &p) const
{
	double xi = (p.y() - _falseNorthing) / (_scale * _rectifyingRadius);
	double eta = (p.x() - _falseEasting) / (_scale * _rectifyingRadius);

	double xiPrim = xi - _delta1 * sin(2*xi) * cosh(2*eta) - _delta2 * sin(4*xi)
	  * cosh(4*eta) - _delta3 * sin(6*xi) * cosh(6*eta) - _delta4 * sin(8*xi)
	  * cosh(8*eta);
	double etaPrim = eta - _delta1 * cos(2*xi) * sinh(2*eta) - _delta2
	  * cos(4*xi) * sinh(4*eta) - _delta3 * cos(6*xi) * sinh(6*eta) - _delta4
	  * cos(8*xi) * sinh(8*eta);

	double phiStar = asin(sin(xiPrim) / cosh(etaPrim));
	double deltaLambda = atan(sinh(etaPrim) / cos(xiPrim));

	double phi = phiStar + sin(phiStar) * cos(phiStar) * (_AStar + _BStar
	  * pow(sin(phiStar), 2) + _CStar * pow(sin(phiStar), 4) + _DStar
	  * pow(sin(phiStar), 6));

	return Coordinates(_centralMeridian + rad2deg(deltaLambda), rad2deg(phi));
}
