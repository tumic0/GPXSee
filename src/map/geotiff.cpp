#include <QFileInfo>
#include <QtEndian>
#include "pcs.h"
#include "latlon.h"
#include "geotiff.h"


#define TIFF_DOUBLE 12
#define TIFF_SHORT  3
#define TIFF_LONG   4

#define ModelPixelScaleTag         33550
#define ModelTiepointTag           33922
#define ModelTransformationTag     34264
#define GeoKeyDirectoryTag         34735
#define GeoDoubleParamsTag         34736
#define ImageWidth                 256
#define ImageHeight                257

#define GTModelTypeGeoKey          1024
#define GTRasterTypeGeoKey         1025
#define GeographicTypeGeoKey       2048
#define GeogGeodeticDatumGeoKey    2050
#define GeogPrimeMeridianGeoKey    2051
#define GeogAngularUnitsGeoKey     2054
#define ProjectedCSTypeGeoKey      3072
#define ProjectionGeoKey           3074
#define ProjCoordTransGeoKey       3075
#define ProjLinearUnitsGeoKey      3076
#define ProjStdParallel1GeoKey     3078
#define ProjStdParallel2GeoKey     3079
#define ProjNatOriginLongGeoKey    3080
#define ProjNatOriginLatGeoKey     3081
#define ProjFalseEastingGeoKey     3082
#define ProjFalseNorthingGeoKey    3083
#define ProjScaleAtNatOriginGeoKey 3092

#define ModelTypeProjected         1
#define ModelTypeGeographic        2
#define ModelTypeGeocentric        3

#define IS_SET(map, key) \
	((map).contains(key) && (map).value(key).SHORT != 32767)

#define ARRAY_SIZE(a) \
	(sizeof(a) / sizeof(*a))

typedef struct {
	quint16 KeyDirectoryVersion;
	quint16 KeyRevision;
	quint16 MinorRevision;
	quint16 NumberOfKeys;
} Header;

typedef struct {
	quint16 KeyID;
	quint16 TIFFTagLocation;
	quint16 Count;
	quint16 ValueOffset;
} KeyEntry;


bool GeoTIFF::readEntry(TIFFFile &file, Ctx &ctx) const
{
	quint16 tag;
	quint16 type;
	quint32 count, offset;

	if (!file.readValue(tag))
		return false;
	if (!file.readValue(type))
		return false;
	if (!file.readValue(count))
		return false;
	if (!file.readValue(offset))
		return false;

	switch (tag) {
		case ModelPixelScaleTag:
			if (type != TIFF_DOUBLE || count != 3)
				return false;
			ctx.scale = offset;
			break;
		case ModelTiepointTag:
			if (type != TIFF_DOUBLE || count < 6 || count % 6)
				return false;
			ctx.tiepoints = offset;
			ctx.tiepointCount = count / 6;
			break;
		case GeoKeyDirectoryTag:
			if (type != TIFF_SHORT)
				return false;
			ctx.keys = offset;
			break;
		case GeoDoubleParamsTag:
			if (type != TIFF_DOUBLE)
				return false;
			ctx.values = offset;
			break;
		case ModelTransformationTag:
			if (type != TIFF_DOUBLE || count != 16)
				return false;
			ctx.matrix = offset;
			break;
	}

	return true;
}

bool GeoTIFF::readIFD(TIFFFile &file, quint32 &offset, Ctx &ctx) const
{
	quint16 count;

	if (!file.seek(offset))
		return false;
	if (!file.readValue(count))
		return false;

	for (quint16 i = 0; i < count; i++)
		if (!readEntry(file, ctx))
			return false;

	if (!file.readValue(offset))
		return false;

	return true;
}

bool GeoTIFF::readScale(TIFFFile &file, quint32 offset, QPointF &scale) const
{
	if (!file.seek(offset))
		return false;

	if (!file.readValue(scale.rx()))
		return false;
	if (!file.readValue(scale.ry()))
		return false;

	return true;
}

bool GeoTIFF::readTiepoints(TIFFFile &file, quint32 offset, quint32 count,
  QList<ReferencePoint> &points) const
{
	double z, pz;
	QPointF xy, pp;

	if (!file.seek(offset))
		return false;

	for (quint32 i = 0; i < count; i++) {
		if (!file.readValue(xy.rx()))
			return false;
		if (!file.readValue(xy.ry()))
			return false;
		if (!file.readValue(z))
			return false;

		if (!file.readValue(pp.rx()))
			return false;
		if (!file.readValue(pp.ry()))
			return false;
		if (!file.readValue(pz))
			return false;

		ReferencePoint p;
		p.xy = xy.toPoint();
		p.pp = pp;
		points.append(p);
	}

	return true;
}

bool GeoTIFF::readMatrix(TIFFFile &file, quint32 offset, double matrix[16]) const
{
	if (!file.seek(offset))
		return false;

	for (int i = 0; i < 16; i++)
		if (!file.readValue(matrix[i]))
			return false;

	return true;
}

bool GeoTIFF::readKeys(TIFFFile &file, Ctx &ctx, QMap<quint16, Value> &kv) const
{
	Header header;
	KeyEntry entry;
	Value value;

	if (!file.seek(ctx.keys))
		return false;

	if (!file.readValue(header.KeyDirectoryVersion))
		return false;
	if (!file.readValue(header.KeyRevision))
		return false;
	if (!file.readValue(header.MinorRevision))
		return false;
	if (!file.readValue(header.NumberOfKeys))
		return false;

	for (int i = 0; i < header.NumberOfKeys; i++) {
		if (!file.readValue(entry.KeyID))
			return false;
		if (!file.readValue(entry.TIFFTagLocation))
			return false;
		if (!file.readValue(entry.Count))
			return false;
		if (!file.readValue(entry.ValueOffset))
			return false;

		switch (entry.KeyID) {
			case GTModelTypeGeoKey:
			case GTRasterTypeGeoKey:
			case GeographicTypeGeoKey:
			case GeogGeodeticDatumGeoKey:
			case GeogPrimeMeridianGeoKey:
			case GeogAngularUnitsGeoKey:
			case ProjectedCSTypeGeoKey:
			case ProjectionGeoKey:
			case ProjCoordTransGeoKey:
			case ProjLinearUnitsGeoKey:
				if (entry.TIFFTagLocation != 0 || entry.Count != 1)
					return false;
				value.SHORT = entry.ValueOffset;
				kv.insert(entry.KeyID, value);
				break;
			case ProjScaleAtNatOriginGeoKey:
			case ProjNatOriginLongGeoKey:
			case ProjNatOriginLatGeoKey:
			case ProjFalseEastingGeoKey:
			case ProjFalseNorthingGeoKey:
			case ProjStdParallel1GeoKey:
			case ProjStdParallel2GeoKey:
				if (!readGeoValue(file, ctx.values, entry.ValueOffset,
				  value.DOUBLE))
					return false;
				kv.insert(entry.KeyID, value);
				break;
			default:
				break;
		}
	}

	return true;
}

bool GeoTIFF::readGeoValue(TIFFFile &file, quint32 offset, quint16 index,
  double &val) const
{
	qint64 pos = file.pos();

	if (!file.seek(offset + index * sizeof(double)))
		return false;
	if (!file.readValue(val))
		return false;

	if (!file.seek(pos))
		return false;

	return true;
}

const GCS *GeoTIFF::gcs(QMap<quint16, Value> &kv)
{
	const GCS *gcs = 0;

	if (IS_SET(kv, GeographicTypeGeoKey)) {
		if (!(gcs = GCS::gcs(kv.value(GeographicTypeGeoKey).SHORT)))
			_errorString = QString("%1: unknown GCS")
			  .arg(kv.value(GeographicTypeGeoKey).SHORT);
	} else if (IS_SET(kv, GeogGeodeticDatumGeoKey)) {
		int pm = IS_SET(kv, GeogPrimeMeridianGeoKey)
		  ? kv.value(GeogPrimeMeridianGeoKey).SHORT : 8901;
		int au = IS_SET(kv, GeogAngularUnitsGeoKey)
		  ? kv.value(GeogAngularUnitsGeoKey).SHORT : 9102;

		if (!(gcs = GCS::gcs(kv.value(GeogGeodeticDatumGeoKey).SHORT, pm, au)))
			_errorString = QString("%1+%2: unknown geodetic datum + prime"
			  " meridian combination")
			  .arg(kv.value(GeogGeodeticDatumGeoKey).SHORT)
			  .arg(kv.value(GeogPrimeMeridianGeoKey).SHORT);
	} else
		_errorString = "Can not determine GCS";

	return gcs;
}

Projection::Method GeoTIFF::method(QMap<quint16, Value> &kv)
{
	if (!IS_SET(kv, ProjCoordTransGeoKey)) {
		_errorString = "Missing coordinate transformation method";
		return Projection::Method();
	}

	switch (kv.value(ProjCoordTransGeoKey).SHORT) {
		case 1:
			return Projection::Method(9807);
		case 7:
			return Projection::Method(1024);
		case 8:
			return Projection::Method(9801);
		case 9:
			return Projection::Method(9802);
		case 10:
			return Projection::Method(9820);
		case 11:
			return Projection::Method(9822);
		default:
			_errorString = QString("%1: unknown coordinate transformation method")
			  .arg(kv.value(ProjCoordTransGeoKey).SHORT);
			return Projection::Method();
	}
}

bool GeoTIFF::projectedModel(QMap<quint16, Value> &kv)
{
	if (IS_SET(kv, ProjectedCSTypeGeoKey)) {
		const PCS *pcs;
		if (!(pcs = PCS::pcs(kv.value(ProjectedCSTypeGeoKey).SHORT))) {
			_errorString = QString("%1: unknown PCS")
			  .arg(kv.value(ProjectedCSTypeGeoKey).SHORT);
			return false;
		}
		_projection = Projection(pcs->gcs(), pcs->method(), pcs->setup(),
		  pcs->units());
	} else if (IS_SET(kv, ProjectionGeoKey)) {
		const GCS *g = gcs(kv);
		if (!g)
			return false;
		const PCS *pcs = PCS::pcs(g, kv.value(ProjectionGeoKey).SHORT);
		if (!pcs) {
			_errorString = QString("%1: unknown projection code")
			  .arg(kv.value(GeographicTypeGeoKey).SHORT)
			  .arg(kv.value(ProjectionGeoKey).SHORT);
			return false;
		}
		_projection = Projection(pcs->gcs(), pcs->method(), pcs->setup(),
		  pcs->units());
	} else {
		const GCS *g = gcs(kv);
		if (!g)
			return false;
		Projection::Method m(method(kv));
		if (m.isNull())
			return false;

		AngularUnits au(IS_SET(kv, GeogAngularUnitsGeoKey)
		  ? kv.value(GeogAngularUnitsGeoKey).SHORT : 9102);
		LinearUnits lu(IS_SET(kv, ProjLinearUnitsGeoKey)
		  ? kv.value(ProjLinearUnitsGeoKey).SHORT : 9001);
		if (lu.isNull()) {
			_errorString = QString("%1: unknown projection linear units code")
			  .arg(kv.value(ProjLinearUnitsGeoKey).SHORT);
			return false;
		}

		Projection::Setup setup(
		  au.toDegrees(kv.value(ProjNatOriginLatGeoKey).DOUBLE),
		  au.toDegrees(kv.value(ProjNatOriginLongGeoKey).DOUBLE),
		  kv.value(ProjScaleAtNatOriginGeoKey).DOUBLE,
		  lu.toMeters(kv.value(ProjFalseEastingGeoKey).DOUBLE),
		  lu.toMeters(kv.value(ProjFalseNorthingGeoKey).DOUBLE),
		  au.toDegrees(kv.value(ProjStdParallel1GeoKey).DOUBLE),
		  au.toDegrees(kv.value(ProjStdParallel2GeoKey).DOUBLE)
		);

		_projection = Projection(g, m, setup, lu);
	}

	return true;
}

bool GeoTIFF::geographicModel(QMap<quint16, Value> &kv)
{
	const GCS *g = gcs(kv);
	if (!g)
		return false;

	_projection = Projection(g);

	return true;
}

bool GeoTIFF::load(const QString &path)
{
	quint32 ifd;
	QList<ReferencePoint> points;
	QPointF scale;
	QMap<quint16, Value> kv;
	Ctx ctx;
	TIFFFile file;


	file.setFileName(path);
	if (!file.open(QIODevice::ReadOnly)) {
		_errorString = QString("Error opening TIFF file: %1")
		  .arg(file.errorString());
		return false;
	}
	if (!file.readHeader(ifd)) {
		_errorString = "Invalid TIFF header";
		return false;
	}

	while (ifd) {
		if (!readIFD(file, ifd, ctx)) {
			_errorString = "Invalid IFD";
			return false;
		}
	}

	if (!ctx.keys) {
		_errorString = "Not a GeoTIFF file";
		return false;
	}

	if (ctx.scale) {
		if (!readScale(file, ctx.scale, scale)) {
			_errorString = "Error reading model pixel scale";
			return false;
		}
	}
	if (ctx.tiepoints) {
		if (!readTiepoints(file, ctx.tiepoints, ctx.tiepointCount, points)) {
			_errorString = "Error reading raster->model tiepoint pairs";
			return false;
		}
	}

	if (!readKeys(file, ctx, kv)) {
		_errorString = "Error reading Geo key/value";
		return false;
	}

	switch (kv.value(GTModelTypeGeoKey).SHORT) {
		case ModelTypeProjected:
			if (!projectedModel(kv))
				return false;
			break;
		case ModelTypeGeographic:
			if (!geographicModel(kv))
				return false;
			break;
		case ModelTypeGeocentric:
			_errorString = "Geocentric models are not supported";
			return false;
		default:
			_errorString = "Unknown/missing model type";
			return false;
	}

	if (ctx.scale && ctx.tiepoints) {
		const ReferencePoint &p = points.first();
		_transform = QTransform(scale.x(), 0, 0, -scale.y(), p.pp.x() - p.xy.x()
		  / scale.x(), p.pp.y() + p.xy.x() / scale.y()).inverted();
	} else if (ctx.tiepointCount > 1) {
		Transform t(points);
		if (t.isNull()) {
			_errorString = t.errorString();
			return false;
		}
		_transform = t.transform();
	} else if (ctx.matrix) {
		double m[16];
		if (!readMatrix(file, ctx.matrix, m)) {
			_errorString = "Error reading transformation matrix";
			return false;
		}
		if (m[2] != 0.0 || m[6] != 0.0 || m[8] != 0.0 || m[9] != 0.0
		  || m[10] != 0.0 || m[11] != 0.0) {
			_errorString = "Not a baseline transformation matrix";
		}
		_transform = QTransform(m[0], m[1], m[4], m[5], m[3], m[7]).inverted();
	} else {
		_errorString = "Incomplete/missing model transformation info";
		return false;
	}

	return true;
}
