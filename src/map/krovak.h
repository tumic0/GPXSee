#ifndef KROVAK_H
#define KROVAK_H

#include "ct.h"

class Ellipsoid;

class Krovak : public CT
{
public:
	Krovak(const Ellipsoid *ellipsoid, double standardParallel,
	  double azimuth, double scale, double centerLatitude,
	  double longitudeOrigin, double falseEasting, double falseNorthing);

	virtual CT *clone() const {return new Krovak(*this);}

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;

private:
	double _e, _a, _b, _t0, _n, _r0, _phiP;
	double _cosAlphaC, _sinAlphaC, _lambda0, _fe, _fn;
};

class KrovakNE : public CT
{
public:
	KrovakNE(const Ellipsoid *ellipsoid, double standardParallel,
	  double azimuth, double scale, double centerLatitude,
	  double longitudeOrigin, double falseEasting, double falseNorthing)
		: _k(ellipsoid, standardParallel, azimuth, scale, centerLatitude,
		longitudeOrigin, falseEasting, falseNorthing) {}

	virtual CT *clone() const {return new KrovakNE(*this);}

	virtual PointD ll2xy(const Coordinates &c) const
	  {PointD p(_k.ll2xy(c)); return PointD(-p.x(), -p.y());}
	virtual Coordinates xy2ll(const PointD &p) const
	  {return _k.xy2ll(PointD(-p.x(), -p.y()));}

private:
	Krovak _k;
};

#endif // KROVAK_H
