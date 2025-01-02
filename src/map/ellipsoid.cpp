#include <QFile>
#include <QDebug>
#include "common/wgs84.h"
#include "common/csv.h"
#include "ellipsoid.h"

QMap<int, Ellipsoid> Ellipsoid::_ellipsoids = defaults();

const Ellipsoid &Ellipsoid::WGS84()
{
	static Ellipsoid e(WGS84_RADIUS, WGS84_FLATTENING);
	return e;
}

QMap<int, Ellipsoid> Ellipsoid::defaults()
{
	QMap<int, Ellipsoid> map;
	map.insert(7030, WGS84());
	return map;
}

const Ellipsoid &Ellipsoid::ellipsoid(int id)
{
	QMap<int, Ellipsoid>::const_iterator it(_ellipsoids.find(id));
	static const Ellipsoid null;

	if (it == _ellipsoids.constEnd())
		return null;
	else
		return it.value();
}

bool Ellipsoid::loadList(const QString &path)
{
	QFile file(path);
	CSV csv(&file);
	QByteArrayList entry;
	bool res;

	if (!file.open(QFile::ReadOnly)) {
		qWarning("%s: %s", qPrintable(path), qPrintable(file.errorString()));
		return false;
	}

	while (!csv.atEnd()) {
		if (!csv.readEntry(entry)) {
			qWarning("%s:%d: Parse error", qPrintable(path), csv.line());
			return false;
		}
		if (entry.size() < 4) {
			qWarning("%s:%d: Invalid column count", qPrintable(path),
			  csv.line() - 1);
			return false;
		}

		int id = entry.at(1).toInt(&res);
		if (!res) {
			qWarning("%s:%d: Invalid ellipsoid code", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		double radius = entry.at(2).toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid radius", qPrintable(path),
			  csv.line() - 1);
			continue;
		}
		double flattening = entry.at(3).toDouble(&res);
		if (!res) {
			qWarning("%s:%d: Invalid flattening", qPrintable(path),
			  csv.line() - 1);
			continue;
		}

		Ellipsoid e(radius, 1.0/flattening);
		_ellipsoids.insert(id, e);
	}

	return true;
}

Ellipsoid::Ellipsoid(double radius, double flattening)
  : _radius(radius), _flattening(flattening)
{
	_es = 2.0 * flattening - flattening * flattening;
	_e2s = (1.0 / (1.0 - _es)) - 1.0;
	_b = radius * (1.0 - flattening);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Ellipsoid &ellipsoid)
{
	dbg.nospace() << "Ellipsoid(" << ellipsoid.radius() << ", "
	  << 1.0 / ellipsoid.flattening() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
