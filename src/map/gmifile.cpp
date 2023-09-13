#include "common/csv.h"
#include "pcs.h"
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

bool GmiFile::parse(QIODevice &device, QList<CalibrationPoint> &points)
{
	int ln = 1;
	int width, height;
	bool ok;

	if (!device.open(QIODevice::ReadOnly)) {
		_errorString = device.errorString();
		return false;
	}

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
			if (!ok || ok <= 0) {
				_errorString = "Invalid image width";
				return false;
			}
		} else if (ln == 4) {
			height = line.toInt(&ok);
			if (!ok || ok <= 0) {
				_errorString = "Invalid image height";
				return false;
			}
			_size = QSize(width, height);
		} else {
			CalibrationPoint cp(calibrationPoint(line));
			if (cp.isValid())
				points.append(cp);
			else
				break;
		}

		ln++;
	}

	device.close();

	return (points.size() >= 2);
}

bool GmiFile::computeTransformation(const QList<CalibrationPoint> &points)
{
	QList<ReferencePoint> rp;
	Projection proj(GCS::WGS84());

	for (int i = 0; i < points.size(); i++)
		rp.append(points.at(i).rp(proj));

	_transform = Transform(rp);
	if (!_transform.isValid()) {
		_errorString = _transform.errorString();
		return false;
	}

	return true;
}

GmiFile::GmiFile(QIODevice &file)
{
	QList<CalibrationPoint> points;

	if (!parse(file, points))
		return;
	if (!computeTransformation(points))
		return;
}
