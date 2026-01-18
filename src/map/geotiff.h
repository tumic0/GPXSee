#ifndef GEOTIFF_H
#define GEOTIFF_H

#include <QMap>
#include <QList>
#include "transform.h"
#include "projection.h"
#include "conversion.h"

class TIFFFile;
class GCS;

class GeoTIFF
{
public:
	GeoTIFF(const QString &path);

	bool isValid() const {return _projection.isValid() && _transform.isValid();}
	const QString &errorString() const {return _errorString;}

	const Projection &projection() const {return _projection;}
	const Transform &transform() const {return _transform;}

private:
	union Value {
		quint16 SHORT;
		double DOUBLE;
	};

	struct Ctx {
		qint64 scale;
		qint64 tiepoints;
		qint64 tiepointCount;
		qint64 matrix;
		qint64 keys;
		qint64 values;

		Ctx() : scale(0), tiepoints(0), tiepointCount(0), matrix(0), keys(0),
		  values(0) {}
	};

	GCS geographicCS(const QMap<quint16, Value> &kv,
	  const QVector<double> &toWGS84);
	Conversion::Method coordinateTransformation(const QMap<quint16, Value> &kv);
	bool geographicModel(const QMap<quint16, Value> &kv,
	  const QVector<double> &toWGS84);
	bool projectedModel(const QMap<quint16, Value> &kv,
	  const QVector<double> &toWGS84);

	static bool readEntry(TIFFFile &file, Ctx &ctx);
	static bool readIFD(TIFFFile &file, qint64 offset, Ctx &ctx);
	static bool readKeys(TIFFFile &file, Ctx &ctx, QMap<quint16, Value> &kv,
	  QVector<double> &toWGS84);
	static bool isWebMercator(const QMap<quint16, Value> &kv);

	Transform _transform;
	Projection _projection;

	QString _errorString;
};

#endif // GEOTIFF_H
