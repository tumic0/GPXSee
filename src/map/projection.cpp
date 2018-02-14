#include "datum.h"
#include "mercator.h"
#include "transversemercator.h"
#include "lambertconic.h"
#include "albersequal.h"
#include "lambertazimuthal.h"
#include "latlon.h"
#include "gcs.h"
#include "projection.h"


Projection::Method::Method(int id)
{
	switch (id) {
		case 1024:
		case 9801:
		case 9802:
		case 9807:
		case 9820:
		case 9822:
		case 9841:
			_id = id;
			break;
		default:
			_id = 0;
	}
}

Projection::Projection(const GCS *gcs, const Method &method, const Setup &setup,
  const LinearUnits &units) : _gcs(gcs), _units(units)
{
	const Ellipsoid *ellipsoid = _gcs->datum().ellipsoid();

	switch (method.id()) {
		case 9807:
			_ct = new TransverseMercator(ellipsoid, setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.scale(), setup.falseEasting(),
			  setup.falseNorthing());
			break;
		case 1024:
		case 9841:
			_ct = new Mercator();
			break;
		case 9802:
			_ct = new LambertConic2(ellipsoid, setup.standardParallel1(),
			  setup.standardParallel2(), setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.falseEasting(),
			  setup.falseNorthing());
			break;
		case 9801:
			_ct = new LambertConic1(ellipsoid, setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.scale(), setup.falseEasting(),
			  setup.falseNorthing());
			break;
		case 9820:
			_ct = new LambertAzimuthal(ellipsoid, setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.falseEasting(),
			  setup.falseNorthing());
			break;
		case 9822:
			_ct = new AlbersEqual(ellipsoid, setup.standardParallel1(),
			  setup.standardParallel2(), setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.falseEasting(),
			  setup.falseNorthing());
			break;
		default:
			_ct = 0;
	}
}

Projection::Projection(const GCS *gcs) : _gcs(gcs)
{
	_ct = new LatLon(gcs->angularUnits());
	_units = LinearUnits(9001);
}

Projection::Projection(const Projection &p)
{
	_gcs = p._gcs;
	_units = p._units;
	_ct = p._ct->clone();
}

Projection::~Projection()
{
	delete _ct;
}

Projection &Projection::operator=(const Projection &p)
{
	_gcs = p._gcs;
	_units = p._units;
	_ct = p._ct->clone();

	return *this;
}

QPointF Projection::ll2xy(const Coordinates &c) const
{
	return _units.fromMeters(_ct->ll2xy(_gcs->fromWGS84(c)));
}

Coordinates Projection::xy2ll(const QPointF &p) const
{
	return _gcs->toWGS84(_ct->xy2ll(_units.toMeters(p)));
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Projection::Setup &setup)
{
	dbg.nospace() << "Setup(" << setup.latitudeOrigin() << ", "
	  << setup.longitudeOrigin() << ", " << setup.scale() << ", "
	  << setup.falseEasting() << ", " << setup.falseNorthing() << ", "
	  << setup.standardParallel1() << ", " << setup.standardParallel2() << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Projection::Method &method)
{
	dbg.nospace() << "Method(" << method.id() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
