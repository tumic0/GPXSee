#include <cmath>
#include <QFile>
#include "common/wgs84.h"
#include "datum.h"


class Datum::Entry {
public:
	Entry(const QString &name, int epsg, const Datum &datum)
	  : _name(name), _epsg(epsg), _datum(datum) {}

	const QString &name() const {return _name;}
	int epsg() const {return _epsg;}
	const Datum &datum() const {return _datum;}

private:
	QString _name;
	int _epsg;
	Datum _datum;
};


// Abridged Molodensky transformation
static Coordinates molodensky(const Coordinates &c, const Datum &from,
  const Datum &to)
{
	double rlat = deg2rad(c.lat());
	double rlon = deg2rad(c.lon());

	double slat = sin(rlat);
	double clat = cos(rlat);
	double slon = sin(rlon);
	double clon = cos(rlon);
	double ssqlat = slat * slat;

	double dx = from.dx() - to.dx();
	double dy = from.dy() - to.dy();
	double dz = from.dz() - to.dz();

	double from_f = from.ellipsoid().flattening();
	double to_f = to.ellipsoid().flattening();
	double df = to_f - from_f;
	double from_a = from.ellipsoid().radius();
	double to_a = to.ellipsoid().radius();
	double da = to_a - from_a;
	double from_esq = from_f * (2.0 - from_f);
	double adb = 1.0 / (1.0 - from_f);
	double rn = from_a / sqrt(1 - from_esq * ssqlat);
	double rm = from_a * (1 - from_esq) / pow((1 - from_esq * ssqlat), 1.5);
	double from_h = 0.0;

	double dlat = (-dx * slat * clon - dy * slat * slon + dz * clat + da
	  * rn * from_esq * slat * clat / from_a + df * (rm * adb + rn / adb) * slat
	  * clat) / (rm + from_h);

	double dlon = (-dx * slon + dy * clon) / ((rn + from_h) * clat);

	return Coordinates(c.lon() + rad2deg(dlon), c.lat() + rad2deg(dlat));
}

QList<Datum::Entry> Datum::_datums = WGS84();
QString Datum::_errorString;
int Datum::_errorLine = 0;

QList<Datum::Entry> Datum::WGS84()
{
	QList<Datum::Entry> list;
	list.append(Datum::Entry("WGS 84", 4326,
	  Datum(Ellipsoid(WGS84_RADIUS, WGS84_FLATTENING), 0.0, 0.0, 0.0)));
	return list;
}

Datum::Datum(const Ellipsoid &ellipsoid, double dx, double dy, double dz)
  : _ellipsoid(ellipsoid), _dx(dx), _dy(dy), _dz(dz)
{
	_WGS84 = (_ellipsoid.radius() == WGS84_RADIUS
	  && _ellipsoid.flattening() == WGS84_FLATTENING
	  && _dx == 0.0 && _dy == 0.0 && _dz == 0.0)
		? true : false;
}

Datum::Datum(int id)
{
	for (int i = 0; i < _datums.size(); i++) {
		if (_datums.at(i).epsg() == id) {
			*this = _datums.at(i).datum();
			return;
		}
	}

	*this = Datum();
}

Datum::Datum(const QString &name)
{
	for (int i = 0; i < _datums.size(); i++) {
		if (_datums.at(i).name() == name) {
			*this = _datums.at(i).datum();
			return;
		}
	}

	*this = Datum();
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

		QString f1 = list[1].trimmed();
		int id = 0;
		if (!f1.isEmpty()) {
			id = f1.toInt(&res);
			if (!res) {
				_errorString = "Invalid datum id";
				return false;
			}
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

		Ellipsoid e(eid);
		if (e.isNull()) {
			_errorString = "Unknown ellipsoid ID";
			return false;
		}

		Datum d(e, dx, dy, dz);
		_datums.append(Entry(list[0].trimmed(), id, d));

		_errorLine++;
	}

	return true;
}

Coordinates Datum::toWGS84(const Coordinates &c) const
{
	return _WGS84 ? c : molodensky(c, *this, Datum(Ellipsoid(WGS84_RADIUS,
	  WGS84_FLATTENING), 0.0, 0.0, 0.0));
}

Coordinates Datum::fromWGS84(const Coordinates &c) const
{
	return _WGS84 ? c : molodensky(c, Datum(Ellipsoid(WGS84_RADIUS,
	  WGS84_FLATTENING), 0.0, 0.0, 0.0), *this);
}

QDebug operator<<(QDebug dbg, const Datum &datum)
{
	dbg.nospace() << "Datum(" << datum.ellipsoid() << ", " << datum.dx() << ", "
	  << datum.dy() << ", " << datum.dz() << ")";
	return dbg.space();
}
