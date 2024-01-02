#include <QIODevice>
#include "gmifile.h"

static CalibrationPoint calibrationPoint(const QByteArray line)
{
	bool xOk, yOk, lonOk, latOk;
	QList<QByteArray> list = line.split(';');
	if (list.size() != 4)
		return CalibrationPoint();

	int x = list.at(0).toInt(&xOk);
	int y = list.at(1).toInt(&yOk);
	double lon = list.at(2).toDouble(&lonOk);
	double lat = list.at(3).toDouble(&latOk);

	return (xOk && yOk && latOk && lonOk)
	  ? CalibrationPoint(PointD(x, y), Coordinates(lon, lat))
	  : CalibrationPoint();
}

bool GmiFile::parse(QIODevice &device)
{
	int ln = 1;
	int width, height;
	bool ok;

	while (!device.atEnd()) {
		QByteArray line = device.readLine(4096);

		if (ln == 1) {
			if (!line.startsWith("Map Calibration data file")) {
				_errorString = "Not a GMI file";
				return false;
			}
		} else if (ln == 2)
			_image = line.trimmed();
		else if (ln == 3) {
			width = line.toInt(&ok);
			if (!ok || width <= 0) {
				_errorString = line + ": invalid image width";
				return false;
			}
		} else if (ln == 4) {
			height = line.toInt(&ok);
			if (!ok || height <= 0) {
				_errorString = line + ": invalid image height";
				return false;
			}
			_size = QSize(width, height);
		} else {
			CalibrationPoint cp(calibrationPoint(line));
			if (cp.isValid())
				_points.append(cp);
			else {
				if (_points.size() < 2) {
					_errorString = line + ": invalid calibration point";
					return false;
				} else
					break;
			}
		}

		ln++;
	}

	if (ln < 6) {
		_errorString = "Unexpected EOF";
		return false;
	}

	return true;
}

GmiFile::GmiFile(QIODevice &file) : _valid(false)
{
	if (!file.open(QIODevice::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	_valid = parse(file);

	file.close();
}
