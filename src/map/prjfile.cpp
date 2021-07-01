#include <QFile>
#include "gcs.h"
#include "pcs.h"
#include "prjfile.h"


#define ARRAY_SIZE(array) \
  (sizeof(array) / sizeof(array[0]))

static Projection::Method projectionMethod(const QString &name)
{
	static const struct {
		int id;
		QString name;
	} methods[] = {
		{1024, "Mercator_Auxiliary_Sphere"},
		{1041, "Krovak"},
		{9801, "Lambert_Conformal_Conic_1SP"},
		{9802, "Lambert_Conformal_Conic_2SP"},
		{9804, "Mercator_1SP"},
		{9807, "Transverse_Mercator"},
		{9809, "Oblique_Stereographic"},
		{9815, "Oblique_Mercator"},
		{9818, "Polyconic"},
		{9820, "Lambert_Azimuthal_Equal_Area"},
		{9822, "Albers_Conic_Equal_Area"},
		{9829, "Polar_Stereographic"}
	};

	for (size_t i = 0; i < ARRAY_SIZE(methods); i++)
		if (!name.compare(methods[i].name, Qt::CaseInsensitive))
			return methods[i].id;

	qWarning("%s: unknown projection", qPrintable(name));

	return Projection::Method();
}

static void setParameter(Projection::Setup *setup, const QString &name,
  double val)
{
	// latitude origin
	if (!name.compare("latitude_of_origin", Qt::CaseInsensitive))
		setup->setLatitudeOrigin(val);
	else if (!name.compare("latitude_of_center", Qt::CaseInsensitive))
		setup->setLatitudeOrigin(val);
	// longitude origin
	else if (!name.compare("central_meridian", Qt::CaseInsensitive))
		setup->setLongitudeOrigin(val);
	else if (!name.compare("longitude_of_center", Qt::CaseInsensitive))
		setup->setLongitudeOrigin(val);
	// scale factor
	else if (!name.compare("scale_factor", Qt::CaseInsensitive))
		setup->setScale(val);
	// false easting
	else if (!name.compare("false_easting", Qt::CaseInsensitive))
		setup->setFalseEasting(val);
	// false northing
	else if (!name.compare("false_northing", Qt::CaseInsensitive))
		setup->setFalseNorthing(val);
	// standard parallel 1
	else if (!name.compare("standard_parallel_1", Qt::CaseInsensitive))
		setup->setStandardParallel1(val);
	else if (!name.compare("pseudo_standard_parallel_1", Qt::CaseInsensitive))
		setup->setStandardParallel1(val);
	// standard parallel 2
	else if (!name.compare("standard_parallel_2", Qt::CaseInsensitive))
		setup->setStandardParallel2(val);
	else if (!name.compare("azimuth", Qt::CaseInsensitive))
		setup->setStandardParallel2(val);
	else
		qWarning("%s: unknown projection parameter", qPrintable(name));
}

PRJFile::Token PRJFile::keyword(CTX &ctx)
{
	static const struct {
		Token token;
		QString name;
	} keywords[] = {
		{COMPD_CS, "COMPD_CS"},
		{PROJCS, "PROJCS"},
		{PROJECTION, "PROJECTION"},
		{PARAMETER, "PARAMETER"},
		{GEOGCS, "GEOGCS"},
		{DATUM, "DATUM"},
		{SPHEROID, "SPHEROID"},
		{PRIMEM, "PRIMEM"},
		{UNIT, "UNIT"},
		{AUTHORITY, "AUTHORITY"},
		{AXIS, "AXIS"},
		{TOWGS84, "TOWGS84"},
		{NORTH, "NORTH"},
		{SOUTH, "SOUTH"},
		{EAST, "EAST"},
		{WEST, "WEST"},
		{UP, "UP"},
		{DOWN, "DOWN"},
		{OTHER, "OTHER"},
		{VERT_CS, "VERT_CS"},
		{VERT_DATUM, "VERT_DATUM"},
		{GEOCCS, "GEOCCS"},
		{FITTED_CS, "FITTED_CS"},
		{LOCAL_CS, "LOCAL_CS"}
	};

	for (size_t i = 0; i < ARRAY_SIZE(keywords); i++)
		if (!ctx.string.compare(keywords[i].name, Qt::CaseInsensitive))
			return keywords[i].token;

	error(ctx);

	return ERROR;
}

void PRJFile::error(CTX &ctx)
{
	if (ctx.token == ERROR)
		return;

	_errorString = QString("parse error on line %1").arg(ctx.line);
	ctx.token = ERROR;
}

int PRJFile::getChar(CTX &ctx)
{
	char c;

	if (ctx.file.getChar(&c))
		return c;
	else
		return -1;
}

void PRJFile::nextToken(CTX &ctx)
{
	int c, state = 0;
	QString flstr;

	while (1) {
		c = getChar(ctx);

		switch (state) {
			case 0:
				if (isspace(c)) {
					if (c == '\n')
						ctx.line++;
					break;
				} if (c == '[') {
					ctx.token = LBRK;
					return;
				}
				if (c == ']') {
					ctx.token = RBRK;
					return;
				}
				if (c == ',') {
					ctx.token = COMMA;
					return;
				}
				if (isalpha(c)) {
					ctx.string = QChar(c);
					state = 2;
					break;
				}
				if (isdigit(c)) {
					flstr += QChar(c);
					state = 3;
					break;
				}
				if (c == '.') {
					flstr += QChar(c);
					state = 4;
					break;
				}
				if (c == '+') {
					flstr += QChar(c);
					state = 3;
					break;
				}
				if (c == '-') {
					flstr += QChar(c);
					state = 3;
					break;
				}
				if (c == '"') {
					ctx.string.clear();
					state = 7;
					break;
				}
				if (c == -1) {
					ctx.token = EOI;
					return;
				}
				error(ctx);
				return;

			case 2:
				if (isalnum(c) || c == '_') {
					ctx.string += QChar(c);
					break;
				}
				ctx.file.ungetChar(c);
				ctx.token = keyword(ctx);
				return;

			case 3:
				if (c == '.') {
					flstr += QChar(c);
					state = 4;
					break;
				}
				if (c == 'e' || c == 'E') {
					flstr += QChar(c);
					state = 5;
					break;
				}
				if (isdigit(c)) {
					flstr += QChar(c);
					break;
				}
				ctx.file.ungetChar(c);
				ctx.number = flstr.toDouble();
				ctx.token = NUMBER;
				return;

			case 4:
				if (isdigit(c)) {
					flstr += QChar(c);
					break;
				}
				if (c == 'e' || c == 'E') {
					flstr += QChar(c);
					state = 5;
					break;
				}
				ctx.file.ungetChar(c);
				ctx.number = flstr.toDouble();
				ctx.token = NUMBER;
				return;

			case 5:
				if (c == '+') {
					flstr += QChar(c);
					state = 6;
					break;
				}
				if (c == '-') {
					flstr += QChar(c);
					state = 6;
					break;
				}
				if (isdigit(c)) {
					flstr += QChar(c);
					state = 6;
					break;
				}
				error(ctx);
				return;

			case 6:
				if (isdigit(c)) {
					flstr += QChar(c);
					break;
				}
				ctx.file.ungetChar(c);
				ctx.number = flstr.toDouble();
				ctx.token = NUMBER;
				return;

			case 7:
				if (c == -1) {
					error(ctx);
					return;
				}
				if (c == '"') {
					ctx.token = STRING;
					return;
				}
				ctx.string += QChar(c);
				break;
		}
	}
}

void PRJFile::compare(CTX &ctx, Token token)
{
	if (ctx.token == token)
		nextToken(ctx);
	else
		error(ctx);
}

void PRJFile::toWGS84(CTX &ctx, double *dx, double *dy, double *dz, double *rx,
  double *ry, double *rz, double *ds)
{
	compare(ctx, TOWGS84);
	compare(ctx, LBRK);
	*dx = ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	*dy = ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	*dz = ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	*rx = -ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	*ry = -ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	*rz = -ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	*ds = ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, RBRK);
}

void PRJFile::optAuthority(CTX &ctx, int *epsg)
{
	if (ctx.token == COMMA) {
		nextToken(ctx);
		authority(ctx, epsg);
	}
}

void PRJFile::authority(CTX &ctx, int *epsg)
{
	bool isEpsg = false;

	compare(ctx, AUTHORITY);
	compare(ctx, LBRK);

	if (!ctx.string.compare("EPSG", Qt::CaseInsensitive))
		isEpsg = true;

	compare(ctx, STRING);
	compare(ctx, COMMA);

	if (isEpsg) {
		bool ok;
		*epsg = ctx.string.toUInt(&ok);
		if (!ok) {
			_errorString = ctx.string + ": invalid EPSG code";
			ctx.token = ERROR;
			return;
		}
	} else
		*epsg = -1;

	compare(ctx, STRING);
	compare(ctx, RBRK);
}

void PRJFile::spheroid(CTX &ctx, Ellipsoid *el)
{
	int epsg = -1;
	double radius, flattening;

	compare(ctx, SPHEROID);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	radius = ctx.number;
	compare(ctx, NUMBER);
	compare(ctx, COMMA);
	flattening = 1.0 / ctx.number;
	compare(ctx, NUMBER);
	optAuthority(ctx, &epsg);
	compare(ctx, RBRK);

	*el = (epsg > 0)
	  ? Ellipsoid::ellipsoid(epsg)
	  : Ellipsoid(radius, flattening);
}

void PRJFile::optDatum2(CTX &ctx, double *dx, double *dy, double *dz, double *rx,
  double *ry, double *rz, double *ds, int *epsg)
{
	switch (ctx.token) {
		case TOWGS84:
			toWGS84(ctx, dx, dy, dz, rx, ry, rz, ds);
			optAuthority(ctx, epsg);
			break;
		case AUTHORITY:
			authority(ctx, epsg);
			break;
		default:
			error(ctx);
	}
}

void PRJFile::optDatum(CTX &ctx, double *dx, double *dy, double *dz, double *rx,
  double *ry, double *rz, double *ds, int *epsg)
{
	if (ctx.token == COMMA) {
		nextToken(ctx);
		optDatum2(ctx, dx, dy, dz, rx, ry, rz, ds, epsg);
	}
}

void PRJFile::datum(CTX &ctx, Datum *dtm, int *epsg)
{
	double dx = NAN, dy = NAN, dz = NAN, rx = NAN, ry = NAN, rz = NAN, ds = NAN;
	Ellipsoid el;

	compare(ctx, DATUM);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	spheroid(ctx, &el);
	optDatum(ctx, &dx, &dy, &dz, &rx, &ry, &rz, &ds, epsg);
	compare(ctx, RBRK);

	*dtm = Datum(el, dx, dy, dz, rx, ry, rz, ds);
}

void PRJFile::unit(CTX &ctx, double *val, int *epsg)
{
	compare(ctx, UNIT);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	*val = ctx.number;
	compare(ctx, NUMBER);
	optAuthority(ctx, epsg);
	compare(ctx, RBRK);
}

void PRJFile::linearUnit(CTX &ctx, LinearUnits *lu)
{
	double val;
	int epsg = -1;

	unit(ctx, &val, &epsg);
	*lu = (epsg > 0) ? LinearUnits(epsg) : LinearUnits(val);
}

void PRJFile::angularUnit(CTX &ctx, AngularUnits *au, int *epsg)
{
	double val;

	unit(ctx, &val, epsg);
	*au = AngularUnits(val);
}

void PRJFile::primeMeridian(CTX &ctx, PrimeMeridian *pm, int *epsg)
{
	double lon;

	compare(ctx, PRIMEM);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	lon = ctx.number;
	compare(ctx, NUMBER);
	optAuthority(ctx, epsg);
	compare(ctx, RBRK);

	*pm = PrimeMeridian(lon);
}

void PRJFile::parameter(CTX &ctx, Projection::Setup *setup)
{
	QString name;

	if (ctx.token == PARAMETER) {
		nextToken(ctx);
		compare(ctx, LBRK);
		name = ctx.string;
		compare(ctx, STRING);
		compare(ctx, COMMA);
		setParameter(setup, name, ctx.number);
		compare(ctx, NUMBER);
		compare(ctx, RBRK);
		compare(ctx, COMMA);
		parameter(ctx, setup);
	}
}

void PRJFile::projection(CTX &ctx, Projection::Method *method)
{
	int epsg = -1;
	QString proj;

	compare(ctx, PROJECTION);
	compare(ctx, LBRK);
	proj = ctx.string;
	compare(ctx, STRING);
	optAuthority(ctx, &epsg);
	compare(ctx, RBRK);

	*method = (epsg > 0) ? Projection::Method(epsg) : projectionMethod(proj);
}

void PRJFile::optProjectedCS2(CTX &ctx, int *epsg)
{
	switch (ctx.token) {
		case AXIS:
			twinAxis(ctx);
			optAuthority(ctx, epsg);
			break;
		case AUTHORITY:
			authority(ctx, epsg);
			break;
		default:
			error(ctx);
	}
}

void PRJFile::optProjectedCS(CTX &ctx, int *epsg)
{
	if (ctx.token == COMMA) {
		nextToken(ctx);
		optProjectedCS2(ctx, epsg);
	}
}

void PRJFile::projectedCS(CTX &ctx, PCS *pcs)
{
	int epsg = -1;
	GCS gcs;
	LinearUnits lu;
	Projection::Method method;
	Projection::Setup setup;

	compare(ctx, PROJCS);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	geographicCS(ctx, &gcs);
	compare(ctx, COMMA);
	projection(ctx, &method);
	compare(ctx, COMMA);
	parameter(ctx, &setup);
	linearUnit(ctx, &lu);
	optProjectedCS(ctx, &epsg);
	compare(ctx, RBRK);

	*pcs = (epsg > 0) ? PCS::pcs(epsg) : PCS(gcs, method, setup, lu);
}

void PRJFile::axisType(CTX &ctx)
{
	switch (ctx.token) {
		case NORTH:
			nextToken(ctx);
			break;
		case SOUTH:
			nextToken(ctx);
			break;
		case EAST:
			nextToken(ctx);
			break;
		case WEST:
			nextToken(ctx);
			break;
		case UP:
			nextToken(ctx);
			break;
		case DOWN:
			nextToken(ctx);
			break;
		case OTHER:
			nextToken(ctx);
			break;
		default:
			error(ctx);
	}
}

void PRJFile::axis(CTX &ctx)
{
	compare(ctx, AXIS);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	axisType(ctx);
	compare(ctx, RBRK);
}

void PRJFile::twinAxis(CTX &ctx)
{
	axis(ctx);
	compare(ctx, COMMA);
	axis(ctx);
}

void PRJFile::optGeographicCS2(CTX &ctx, int *epsg)
{
	switch (ctx.token) {
		case AXIS:
			twinAxis(ctx);
			optAuthority(ctx, epsg);
			break;
		case AUTHORITY:
			authority(ctx, epsg);
			break;
		default:
			error(ctx);
	}
}

void PRJFile::optGeographicCS(CTX &ctx, int *epsg)
{
	if (ctx.token == COMMA) {
		nextToken(ctx);
		optGeographicCS2(ctx, epsg);
	}
}

void PRJFile::geographicCS(CTX &ctx, GCS *gcs)
{
	int gcsEpsg = -1, datumEpsg = -1, pmEpsg = -1, auEpsg = -1;
	PrimeMeridian pm;
	AngularUnits au;
	Datum dat;

	compare(ctx, GEOGCS);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	datum(ctx, &dat, &datumEpsg);
	compare(ctx, COMMA);
	primeMeridian(ctx, &pm, &pmEpsg);
	compare(ctx, COMMA);
	angularUnit(ctx, &au, &auEpsg);
	optGeographicCS(ctx, &gcsEpsg);
	compare(ctx, RBRK);

	*gcs = (gcsEpsg > 0)
	  ? GCS::gcs(gcsEpsg)
	  : (datumEpsg > 0 && pmEpsg > 0 && auEpsg > 0)
		 ? GCS::gcs(datumEpsg, pmEpsg, auEpsg)
		 : GCS(dat, pm, au);
}

void PRJFile::horizontalCS(CTX &ctx)
{
	switch (ctx.token) {
		case PROJCS:
			{PCS pcs;
			projectedCS(ctx, &pcs);
			_projection = Projection(pcs);}
			break;
		case GEOGCS:
			{GCS gcs;
			geographicCS(ctx, &gcs);
			_projection = Projection(gcs);}
			break;
		default:
			error(ctx);
	}
}

void PRJFile::verticalDatum(CTX &ctx)
{
	int epsg;

	compare(ctx, VERT_DATUM);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	compare(ctx, NUMBER);
	optAuthority(ctx, &epsg);
	compare(ctx, RBRK);
}

void PRJFile::optVerticalCS2(CTX &ctx, int *epsg)
{
	switch (ctx.token) {
		case AXIS:
			axis(ctx);
			optAuthority(ctx, epsg);
			break;
		case AUTHORITY:
			authority(ctx, epsg);
			break;
		default:
			error(ctx);
	}
}

void PRJFile::optVerticalCS(CTX &ctx, int *epsg)
{
	if (ctx.token == COMMA) {
		nextToken(ctx);
		optVerticalCS2(ctx, epsg);
	}
}

void PRJFile::verticalCS(CTX &ctx)
{
	int luEpsg, vcsEpsg;
	double lu;

	compare(ctx, VERT_CS);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	verticalDatum(ctx);
	compare(ctx, COMMA);
	unit(ctx, &lu, &luEpsg);
	optVerticalCS(ctx, &vcsEpsg);
	compare(ctx, RBRK);
}

void PRJFile::optCS(CTX &ctx, int *epsg)
{
	if (ctx.token == COMMA) {
		nextToken(ctx);
		authority(ctx, epsg);
	}
}

void PRJFile::compdCS(CTX &ctx)
{
	int epsg = -1;

	compare(ctx, COMPD_CS);
	compare(ctx, LBRK);
	compare(ctx, STRING);
	compare(ctx, COMMA);
	CS(ctx);
	compare(ctx, COMMA);
	CS(ctx);
	optCS(ctx, &epsg);
	compare(ctx, RBRK);
}

void PRJFile::CS(CTX &ctx)
{
	switch (ctx.token) {
		case COMPD_CS:
			compdCS(ctx);
			break;
		case PROJCS:
		case GEOGCS:
			horizontalCS(ctx);
			break;
		case VERT_CS:
			verticalCS(ctx);
			break;
		case GEOCCS:
			_errorString = "geocentric coordinate systems not supported";
			ctx.token = ERROR;
			break;
		case FITTED_CS:
			_errorString = "fitted coordinate systems not supported";
			ctx.token = ERROR;
			break;
		case LOCAL_CS:
			_errorString = "local coordinate systems not supported";
			ctx.token = ERROR;
			break;
		default:
			error(ctx);
	}
}

PRJFile::PRJFile(const QString &fileName)
{
	CTX ctx(fileName);

	if (!ctx.file.open(QIODevice::ReadOnly)) {
		_errorString = ctx.file.errorString();
		return;
	}

	nextToken(ctx);
	CS(ctx);

	if (ctx.token == EOI) {
		if (!_projection.isValid())
			_errorString = "unknown/incomplete projection";
	} else {
		error(ctx);
		_projection = Projection();
	}
}
