#include "datum.h"
#include "mercator.h"
#include "transversemercator.h"
#include "lambertconic.h"
#include "albersequal.h"
#include "lambertazimuthal.h"
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

Projection *Projection::projection(const Datum &datum, const Method &method,
  const Setup &setup)
{
	const Ellipsoid &ellipsoid = datum.ellipsoid();

	switch (method.id()) {
		case 9807:
			return new TransverseMercator(ellipsoid, setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.scale(), setup.falseEasting(),
			  setup.falseNorthing());
		case 1024:
		case 9841:
			return new Mercator();
		case 9802:
			return new LambertConic2(ellipsoid, setup.standardParallel1(),
			  setup.standardParallel2(), setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.falseEasting(),
			  setup.falseNorthing());
		case 9801:
			return new LambertConic1(ellipsoid, setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.scale(), setup.falseEasting(),
			  setup.falseNorthing());
		case 9820:
			return new LambertAzimuthal(ellipsoid, setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.falseEasting(),
			  setup.falseNorthing());
		case 9822:
			return new AlbersEqual(ellipsoid, setup.standardParallel1(),
			  setup.standardParallel2(), setup.latitudeOrigin(),
			  setup.longitudeOrigin(), setup.falseEasting(),
			  setup.falseNorthing());
		default:
			return 0;
	}
}

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
