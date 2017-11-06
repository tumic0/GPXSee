#include <cmath>
#include <QFile>
#include "wgs84.h"
#include "rd.h"
#include "datum.h"


static QMap<QString, Datum> WGS84()
{
	QMap<QString, Datum> map;
	map.insert("WGS 84", Datum(Ellipsoid(WGS84_RADIUS, WGS84_FLATTENING),
	  0.0, 0.0, 0.0));
	return map;
}

QMap<QString, Datum> Datum::_datums = WGS84();
QString Datum::_errorString;
int Datum::_errorLine = 0;

Datum Datum::datum(const QString &name)
{
	QMap<QString, Datum>::const_iterator it = _datums.find(name);

	if (it == _datums.end())
		return Datum();

	return it.value();
}

bool Datum::loadList(const QString &path)
{
	QFile file(path);
	bool res;

	if (!file.open(QFile::ReadOnly)) {
		_errorString = qPrintable(file.errorString());
		return false;
	}

	_errorLine = 1;
	_errorString.clear();

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 6) {
			_errorString = "Format error";
			return false;
		}

		int eid = list[2].trimmed().toInt(&res);
		if (!res) {
			_errorString = "Invalid ellipsoid id";
			return false;
		}
		double dx = list[3].trimmed().toDouble(&res);
		if (!res) {
			_errorString = "Invalid dx";
			return false;
		}
		double dy = list[4].trimmed().toDouble(&res);
		if (!res) {
			_errorString = "Invalid dy";
			return false;
		}
		double dz = list[5].trimmed().toDouble(&res);
		if (!res) {
			_errorString = "Invalid dz";
			return false;
		}

		Ellipsoid e = Ellipsoid::ellipsoid(eid);
		if (e.isNull()) {
			_errorString = "Unknown ellipsoid ID";
			return false;
		}

		Datum d(e, dx, dy, dz);
		_datums.insert(list[0].trimmed(), d);

		_errorLine++;
	}

	return true;
}

// Abridged Molodensky transformation
Coordinates Datum::toWGS84(const Coordinates &c) const
{
	if (_ellipsoid.radius() == WGS84_RADIUS
	  && _ellipsoid.flattening() == WGS84_FLATTENING
	  && _dx == 0.0 && _dy == 0.0 && _dz == 0.0)
		return c;

	double rlat = deg2rad(c.lat());
	double rlon = deg2rad(c.lon());

	double slat = sin(rlat);
	double clat = cos(rlat);
	double slon = sin(rlon);
	double clon = cos(rlon);
	double ssqlat = slat * slat;

	double from_f = ellipsoid().flattening();
	double df = WGS84_FLATTENING - from_f;
	double from_a = ellipsoid().radius();
	double da = WGS84_RADIUS - from_a;
	double from_esq = ellipsoid().flattening()
	  * (2.0 - ellipsoid().flattening());
	double adb = 1.0 / (1.0 - from_f);
	double rn = from_a / sqrt(1 - from_esq * ssqlat);
	double rm = from_a * (1 - from_esq) / pow((1 - from_esq * ssqlat), 1.5);
	double from_h = 0.0;

	double dlat = (-dx() * slat * clon - dy() * slat * slon + dz() * clat + da
	  * rn * from_esq * slat * clat / from_a + df * (rm * adb + rn / adb) * slat
	  * clat) / (rm + from_h);
	double dlon = (-dx() * slon + dy() * clon) / ((rn + from_h) * clat);

	return Coordinates(c.lon() + rad2deg(dlon), c.lat() + rad2deg(dlat));
}
