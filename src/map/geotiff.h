#ifndef GEOTIFF_H
#define GEOTIFF_H

#include <QCoreApplication>
#include <QTransform>
#include <QFile>
#include <QMap>
#include "gcs.h"
#include "projection.h"
#include "tifffile.h"
#include "transform.h"
#include "linearunits.h"

class GeoTIFF
{
	Q_DECLARE_TR_FUNCTIONS(GeoTIFF)

public:
	bool load(const QString &path);
	const QString &errorString() const {return _errorString;}

	const Projection &projection() const {return _projection;}
	const QTransform &transform() const {return _transform;}

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

	bool readEntry(TIFFFile &file, Ctx &ctx) const;
	bool readIFD(TIFFFile &file, quint32 &offset, Ctx &ctx) const;
	bool readScale(TIFFFile &file, quint32 offset, QPointF &scale) const;
	bool readTiepoints(TIFFFile &file, quint32 offset, quint32 count,
	  QList<ReferencePoint> &points) const;
	bool readMatrix(TIFFFile &file, quint32 offset, double matrix[16]) const;
	bool readKeys(TIFFFile &file, Ctx &ctx, QMap<quint16, Value> &kv) const;
	bool readGeoValue(TIFFFile &file, quint32 offset, quint16 index,
	  double &val) const;

	const GCS *gcs(QMap<quint16, Value> &kv);
	Projection::Method method(QMap<quint16, Value> &kv);
	bool geographicModel(QMap<quint16, Value> &kv);
	bool projectedModel(QMap<quint16, Value> &kv);

	QTransform _transform;
	Projection _projection;

	QString _errorString;
};

#endif // GEOTIFF_H
