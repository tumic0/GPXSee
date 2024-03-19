#ifndef PROJECTION_H
#define PROJECTION_H

#include <QtGlobal>
#include "common/coordinates.h"
#include "pointd.h"
#include "linearunits.h"
#include "coordinatesystem.h"
#include "gcs.h"
#include "ct.h"
#include "proj/latlon.h"

class PCS;

class Projection {
public:
	Projection() : _ct(0) {}
	Projection(const Projection &p);
	Projection(const PCS &pcs);
	Projection(const GCS &gcs, const CoordinateSystem &cs
	  = CoordinateSystem(CoordinateSystem::YX));
	~Projection();

	Projection &operator=(const Projection &p);
	bool operator==(const Projection &p) const;

	bool isNull() const
	{
		return (_gcs.isNull() && _ct == 0 && _units.isNull() && _cs.isNull());
	}
	bool isValid() const
	{
		/* We do not check the CoordinateSystem here as it is not always defined
		   and except of WMTS/WMS it is not needed. */
		return (_gcs.isValid() && _ct != 0 && _units.isValid());
	}
	bool isGeographic() const
	{
		return (dynamic_cast<const LatLon*>(_ct) != 0);
	}

	PointD ll2xy(const Coordinates &c) const
	{
		Q_ASSERT(isValid());
		return _units.fromMeters(_ct->ll2xy(_gcs.fromWGS84(c)));
	}
	Coordinates xy2ll(const PointD &p) const
	{
		Q_ASSERT(isValid());
		return _gcs.toWGS84(_ct->xy2ll(_units.toMeters(p)));
	}

	const LinearUnits &units() const {return _units;}
	const CoordinateSystem &coordinateSystem() const {return _cs;}

private:
	GCS _gcs;
	const CT *_ct;
	LinearUnits _units;
	CoordinateSystem _cs;
};

#endif // PROJECTION_H
