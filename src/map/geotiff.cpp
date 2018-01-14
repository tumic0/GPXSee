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
#define GeogEllipsoidGeoKey        2056
#define ProjectedCSTypeGeoKey      3072
#define ProjectionGeoKey           3074
#define ProjCoordTransGeoKey       3075
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
			case GeographicTypeGeoKey:
			case ProjectedCSTypeGeoKey:
			case ProjCoordTransGeoKey:
			case GTModelTypeGeoKey:
			case GTRasterTypeGeoKey:
			case GeogGeodeticDatumGeoKey:
			case ProjectionGeoKey:
			case GeogEllipsoidGeoKey:
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

Datum GeoTIFF::datum(QMap<quint16, Value> &kv)
{
	Datum datum;

	if (IS_SET(kv, GeographicTypeGeoKey)) {
		datum = Datum(kv.value(GeographicTypeGeoKey).SHORT);
		if (datum.isNull())
			_errorString = QString("%1: unknown GCS")
			  .arg(kv.value(GeographicTypeGeoKey).SHORT);
	} else if (IS_SET(kv, GeogGeodeticDatumGeoKey)) {
		datum = Datum(kv.value(GeogGeodeticDatumGeoKey).SHORT - 2000);
		if (datum.isNull())
			_errorString = QString("%1: unknown geodetic datum")
			  .arg(kv.value(GeogGeodeticDatumGeoKey).SHORT);
	} else if (IS_SET(kv, GeogEllipsoidGeoKey)
	  && kv.value(GeogEllipsoidGeoKey).SHORT == 7019) {
		datum = Datum(4326);
	} else
		_errorString = "Missing datum";

	return datum;
}

Projection::Method GeoTIFF::method(QMap<quint16, Value> &kv)
{
	int epsg;
	int table[] = {0, 9807, 0, 0, 0, 0, 0, 1024, 9801, 9802, 9820,
	  9822};

	if (!IS_SET(kv, ProjCoordTransGeoKey)) {
		_errorString = "Missing coordinate transformation method";
		return Projection::Method();
	}

	quint16 index = kv.value(ProjCoordTransGeoKey).SHORT;
	epsg = (index >= ARRAY_SIZE(table)) ? 0 : table[index];

	if (!epsg) {
		_errorString = QString("Unknown coordinate transformation method");
		return Projection::Method();
	}

	return Projection::Method(epsg);
}

bool GeoTIFF::projectedModel(QMap<quint16, Value> &kv)
{
	PCS pcs;

	if (IS_SET(kv, ProjectedCSTypeGeoKey)) {
		pcs = PCS(kv.value(ProjectedCSTypeGeoKey).SHORT);
		if (pcs.isNull())
			_errorString = QString("%1: unknown PCS")
			  .arg(kv.value(ProjectedCSTypeGeoKey).SHORT);
	} else if (IS_SET(kv, GeographicTypeGeoKey)
	  && IS_SET(kv, ProjectionGeoKey)) {
		pcs = PCS(kv.value(GeographicTypeGeoKey).SHORT,
		  kv.value(ProjectionGeoKey).SHORT);
		if (pcs.isNull())
			_errorString = QString("%1+%2: unknown GCS+projection combination")
			  .arg(kv.value(GeographicTypeGeoKey).SHORT)
			  .arg(kv.value(ProjectionGeoKey).SHORT);
	} else if (IS_SET(kv, GeogGeodeticDatumGeoKey)
	  && IS_SET(kv, ProjectionGeoKey)) {
		pcs = PCS(kv.value(GeogGeodeticDatumGeoKey).SHORT - 2000,
		  kv.value(ProjectionGeoKey).SHORT);
		if (pcs.isNull())
			_errorString =
			  QString("%1+%2: unknown geodetic datum+projection combination")
			  .arg(kv.value(GeogGeodeticDatumGeoKey).SHORT)
			  .arg(kv.value(ProjectionGeoKey).SHORT);
	} else {
		Datum d = datum(kv);
		if (d.isNull())
			return false;
		Projection::Method m = method(kv);
		if (m.isNull())
			return false;

		Projection::Setup setup(
		  kv.value(ProjNatOriginLatGeoKey).DOUBLE,
		  kv.value(ProjNatOriginLongGeoKey).DOUBLE,
		  kv.value(ProjScaleAtNatOriginGeoKey).DOUBLE,
		  kv.value(ProjFalseEastingGeoKey).DOUBLE,
		  kv.value(ProjFalseNorthingGeoKey).DOUBLE,
		  kv.value(ProjStdParallel1GeoKey).DOUBLE,
		  kv.value(ProjStdParallel2GeoKey).DOUBLE
		);

		pcs = PCS(d, m, setup);
	}

	_datum = pcs.datum();
	_projection = Projection::projection(pcs.datum(), pcs.method(),
	  pcs.setup());

	return true;
}

bool GeoTIFF::geographicModel(QMap<quint16, Value> &kv)
{
	_datum = datum(kv);
	if (_datum.isNull())
		return false;

	_projection = new LatLon();

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
