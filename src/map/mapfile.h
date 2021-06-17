#ifndef MAPFILE_H
#define MAPFILE_H

#include "transform.h"
#include "projection.h"
#include "calibrationpoint.h"

class QIODevice;
class GCS;

class MapFile
{
public:
	MapFile(QIODevice &file);

	bool isValid() const
	  {return !_image.isNull() && _projection.isValid() && _transform.isValid();}
	const QString &errorString() const {return _errorString;}

	const Projection &projection() const {return _projection;}
	const Transform &transform() const {return _transform;}

	const QString &name() const {return _name;}
	const QString &image() const {return _image;}
	const QSize &size() const {return _size;}

private:
	int parse(QIODevice &device, QList<CalibrationPoint> &points,
	  QString &projection, Projection::Setup &setup, QString &datum);
	bool parseMapFile(QIODevice &device, QList<CalibrationPoint> &points,
	  QString &projection, Projection::Setup &setup, QString &datum);
	bool createProjection(const QString &datum, const QString &projection,
	  const Projection::Setup &setup);
	bool computeTransformation(const QList<CalibrationPoint> &points);

	QString _name;
	QString _image;
	QSize _size;

	Projection _projection;
	Transform _transform;

	QString _errorString;
};

#endif // MAPFILE_H
