#ifndef MAPFILE_H
#define MAPFILE_H

#include <QIODevice>
#include <QTransform>
#include "datum.h"
#include "transform.h"
#include "projection.h"

class MapFile
{
public:
	bool load(QIODevice &file);

	const Datum &datum() const {return _datum;}
	Projection *projection() const {return _projection;}
	const QTransform &transform() const {return _transform;}

	const QString &name() const {return _name;}
	const QString &image() const {return _image;}
	const QSize &size() const {return _size;}

	const QString &errorString() const {return _errorString;}

private:
	struct CalibrationPoint {
		ReferencePoint rp;
		Coordinates ll;
		int zone;
	};

	int parse(QIODevice &device, QList<CalibrationPoint> &points,
	  QString &projection, Projection::Setup &setup, QString &datum);
	bool parseMapFile(QIODevice &device, QList<CalibrationPoint> &points,
	  QString &projection, Projection::Setup &setup, QString &datum);
	bool createDatum(const QString &datum);
	bool createProjection(const QString &projection,
	  const Projection::Setup &setup, QList<CalibrationPoint> &points);
	bool computeTransformation(QList<CalibrationPoint> &points);

	QString _name;
	QString _image;
	QSize _size;

	Datum _datum;
	QTransform _transform;
	Projection *_projection;

	QString _errorString;
};

#endif // MAPFILE_H
