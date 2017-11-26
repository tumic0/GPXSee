#include "latlon.h"
#include "mercator.h"
#include "transversemercator.h"
#include "utm.h"
#include "lambertconic.h"
#include "albersequal.h"
#include "lambertazimuthal.h"


QString Projection::_errorString;

Projection *Projection::projection(const QString &name,
  const Ellipsoid &ellipsoid, const Setup &setup)
{
	if (setup.latitudeOrigin < -90.0 || setup.latitudeOrigin > 90.0
	  || setup.longitudeOrigin < -180.0 || setup.longitudeOrigin > 180.0
	  || setup.standardParallel1 < -90.0 || setup.standardParallel1 > 90.0
	  || setup.standardParallel2 < -90.0 || setup.standardParallel2 > 90.0) {
		_errorString = "Invalid projection setup";
		return 0;
	}

	if (name == "Mercator")
		return new Mercator();
	else if (name == "Transverse Mercator")
		return new TransverseMercator(ellipsoid,
		  setup.latitudeOrigin, setup.longitudeOrigin, setup.scale,
		  setup.falseEasting, setup.falseNorthing);
	else if (name == "Latitude/Longitude")
		return new LatLon();
	else if (name == "Lambert Conformal Conic")
		return new LambertConic(ellipsoid,
		  setup.standardParallel1, setup.standardParallel2,
		  setup.latitudeOrigin, setup.longitudeOrigin, setup.scale,
		  setup.falseEasting, setup.falseNorthing);
	else if (name == "Albers Equal Area")
		return new AlbersEqual(ellipsoid, setup.standardParallel1,
		  setup.standardParallel2, setup.latitudeOrigin, setup.longitudeOrigin,
		  setup.falseEasting, setup.falseNorthing);
	else if (name == "(A)Lambert Azimuthual Equal Area")
		return new LambertAzimuthal(ellipsoid, setup.latitudeOrigin,
		  setup.longitudeOrigin, setup.falseEasting, setup.falseNorthing);
	else if (name == "(UTM) Universal Transverse Mercator") {
		if (setup.zone)
			return new UTM(ellipsoid, setup.zone);
		else {
			_errorString = "Can not determine UTM zone";
			return 0;
		}
	} else if (name == "(NZTM2) New Zealand TM 2000")
		return new TransverseMercator(ellipsoid, 0, 173.0, 0.9996,
		  1600000, 10000000);
	else if (name == "(BNG) British National Grid")
		return new TransverseMercator(ellipsoid, 49, -2, 0.999601,
		  400000, -100000);
	else if (name == "(IG) Irish Grid")
		return new TransverseMercator(ellipsoid, 53.5, -8, 1.000035,
		  200000, 250000);
	else if (name == "(SG) Swedish Grid")
		return new TransverseMercator(ellipsoid, 0, 15.808278, 1,
		  1500000, 0);
	else if (name == "(I) France Zone I")
		return new LambertConic(ellipsoid, 48.598523, 50.395912,
		  49.5, 2.337229, 1 /*0.99987734*/, 600000, 1200000);
	else if (name == "(II) France Zone II")
		return new LambertConic(ellipsoid, 45.898919, 47.696014,
		  46.8, 2.337229, 1 /*0.99987742*/, 600000, 2200000);
	else if (name == "(III) France Zone III")
		return new LambertConic(ellipsoid, 43.199291, 44.996094,
		  44.1, 2.337229, 1 /*0.99987750*/, 600000, 3200000);
	else if (name == "(IV) France Zone IV")
		return new LambertConic(ellipsoid, 41.560388, 42.767663,
		  42.165, 2.337229, 1 /*0.99994471*/, 234.358, 4185861.369);
	else if (name == "(VICGRID) Victoria Australia")
		return new LambertConic(ellipsoid, -36, -38, -37, 145, 1,
		  2500000, 4500000);
	else if (name == "(VG94) VICGRID94 Victoria Australia")
		return new LambertConic(ellipsoid, -36, -38, -37, 145, 1,
		  2500000, 2500000);
	else {
		_errorString = QString("%1: Unknown map projection").arg(name);
		return 0;
	}
}

const QString &Projection::errorString()
{
	return _errorString;
}
