#ifndef GEOTIFF_H
#define GEOTIFF_H

#include <QTransform>
#include <QFile>
#include <QMap>
#include "datum.h"
#include "projection.h"
#include "tifffile.h"
#include "transform.h"

class GeoTIFF
{
public:
	GeoTIFF() : _projection(0) {}

	bool load(const QString &path);

	const Datum &datum() const {return _datum;}
	Projection *projection() const {return _projection;}
	const QTransform &transform() const {return _transform;}

	const QString &errorString() const {return _errorString;}

private:
	union Value {
		quint16 SHORT;
		double DOUBLE;
	};

	struct Ctx {
		quint32 scale;
		quint32 tiepoints;
		quint32 tiepointCount;
		quint32 matrix;
		quint32 keys;
		quint32 values;

		Ctx() : scale(0), tiepoints(0), tiepointCount(0), matrix(0), keys(0),
		  values(0) {}
	};

	bool readEntry(TIFFFile &file, Ctx &ctx);
	bool readIFD(TIFFFile &file, quint32 &offset, Ctx &ctx);
	bool readScale(TIFFFile &file, quint32 offset, QPointF &scale);
	bool readTiepoints(TIFFFile &file, quint32 offset, quint32 count,
	  QList<ReferencePoint> &points);
	bool readKeys(TIFFFile &file, Ctx &ctx, QMap<quint16, Value> &kv);
	bool readGeoValue(TIFFFile &file, quint32 offset, quint16 index,
	  double &val);

	Datum datum(QMap<quint16, Value> &kv);
	Projection::Method method(QMap<quint16, Value> &kv);
	bool geographicModel(QMap<quint16, Value> &kv);
	bool projectedModel(QMap<quint16, Value> &kv);

	QTransform _transform;
	Datum _datum;
	Projection *_projection;

	QString _errorString;
};

#endif // GEOTIFF_H
