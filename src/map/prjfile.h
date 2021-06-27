#ifndef PRJFILE_H
#define PRJFILE_H

#include <QFile>
#include "projection.h"

class Datum;
class Ellipsoid;
class PrimeMeridian;

class PRJFile
{
public:
	PRJFile(const QString &fileName);

	const Projection &projection() const {return _projection;}
	const QString &errorString() const {return _errorString;}

private:
	enum Token {
		START,		/* Initial value */
		EOI,		/* End of File */
		ERROR,		/* Parse error */

		NUMBER,		/* Real number */
		STRING,		/* String */

		LBRK,		/* '[' */
		RBRK,		/* ']' */
		COMMA,		/* ',' */

		/* Keywords */
		COMPD_CS, PROJCS, PROJECTION, GEOGCS, DATUM, SPHEROID, PRIMEM, UNIT,
		AUTHORITY, AXIS, TOWGS84, PARAMETER, NORTH, SOUTH, EAST, WEST, UP, DOWN,
		OTHER, VERT_CS, VERT_DATUM, GEOCCS, FITTED_CS, LOCAL_CS
	};

	struct CTX {
		CTX(const QString &fileName) : file(fileName), token(START), line(1) {}

		QFile file;
		Token token;
		QString string;
		double number;
		int line;
	};

	Token keyword(CTX &ctx);
	int getChar(CTX &ctx);
	void error(CTX &ctx);
	void nextToken(CTX &ctx);
	void compare(CTX &ctx, Token token);

	void CS(CTX &ctx);
	void horizontalCS(CTX &ctx);
	void verticalCS(CTX &ctx);
	void geographicCS(CTX &ctx, GCS *gcs);
	void projectedCS(CTX &ctx, PCS *pcs);
	void compdCS(CTX &ctx);
	void projection(CTX &ctx, Projection::Method *method);
	void parameter(CTX &ctx, Projection::Setup *setup);
	void datum(CTX &ctx, Datum *dtm, int *epsg);
	void verticalDatum(CTX &ctx);
	void unit(CTX &ctx, double *val, int *epsg);
	void angularUnit(CTX &ctx, AngularUnits *au, int *epsg);
	void primeMeridian(CTX &ctx, PrimeMeridian *pm, int *epsg);
	void linearUnit(CTX &ctx, LinearUnits *lu);
	void spheroid(CTX &ctx, Ellipsoid *el);
	void authority(CTX &ctx, int *epsg);
	void toWGS84(CTX &ctx, double *dx, double *dy, double *dz, double *rx,
	  double *ry, double *rz, double *ds);
	void twinAxis(CTX &ctx);
	void axis(CTX &ctx);
	void axisType(CTX &ctx);
	void optAuthority(CTX &ctx, int *epsg);
	void optDatum(CTX &ctx, double *dx, double *dy, double *dz, double *rx,
	  double *ry, double *rz, double *ds, int *epsg);
	void optDatum2(CTX &ctx, double *dx, double *dy, double *dz, double *rx,
	  double *ry, double *rz, double *ds, int *epsg);
	void optGeographicCS(CTX &ctx, int *epsg);
	void optGeographicCS2(CTX &ctx, int *epsg);
	void optProjectedCS(CTX &ctx, int *epsg);
	void optProjectedCS2(CTX &ctx, int *epsg);
	void optCS(CTX &ctx, int *epsg);
	void optVerticalCS(CTX &ctx, int *epsg);
	void optVerticalCS2(CTX &ctx, int *epsg);

	Projection _projection;
	QString _errorString;
};

#endif // PRJFILE_H
