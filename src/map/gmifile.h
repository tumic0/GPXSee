#ifndef GMIFILE_H
#define GMIFILE_H

#include "transform.h"
#include "calibrationpoint.h"

class QIODevice;
class GCS;

class GmiFile
{
public:
	GmiFile(QIODevice &file);

	bool isValid() const {return !_image.isNull() && _transform.isValid();}
	const QString &errorString() const {return _errorString;}

	const Transform &transform() const {return _transform;}
	const QString &image() const {return _image;}
	const QSize &size() const {return _size;}

private:
	bool parse(QIODevice &device, QList<CalibrationPoint> &points);
	bool computeTransformation(const QList<CalibrationPoint> &points);

	QString _image;
	QSize _size;
	Transform _transform;

	QString _errorString;
};

#endif // GMIFILE_H
