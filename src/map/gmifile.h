#ifndef GMIFILE_H
#define GMIFILE_H

#include "calibrationpoint.h"

class QIODevice;

class GmiFile
{
public:
	GmiFile(QIODevice &file);

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

	const QString &image() const {return _image;}
	const QSize &size() const {return _size;}
	const QList<CalibrationPoint> &calibrationPoints() const {return _points;}

private:
	bool parse(QIODevice &device);

	QString _image;
	QSize _size;
	QList<CalibrationPoint> _points;

	bool _valid;
	QString _errorString;
};

#endif // GMIFILE_H
