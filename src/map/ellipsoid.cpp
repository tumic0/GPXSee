#include <QFile>
#include <QDebug>
#include "common/wgs84.h"
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

const Ellipsoid *Ellipsoid::ellipsoid(int id)
{
	QMap<int, Ellipsoid>::const_iterator it(_ellipsoids.find(id));

	if (it == _ellipsoids.constEnd())
		return 0;
	else
		return &(it.value());
}

void Ellipsoid::loadList(const QString &path)
{
	QFile file(path);
	bool res;
	int ln = 0;


	if (!file.open(QFile::ReadOnly)) {
		qWarning("Error opening ellipsoids file: %s: %s", qPrintable(path),
		  qPrintable(file.errorString()));
		return;
	}

	while (!file.atEnd()) {
		ln++;

		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 4) {
			qWarning("%s: %d: Format error", qPrintable(path), ln);
			continue;
		}

		int id = list[1].trimmed().toInt(&res);
		if (!res) {
			qWarning("%s: %d: Invalid ellipsoid code", qPrintable(path), ln);
			continue;
		}
		double radius = list[2].trimmed().toDouble(&res);
		if (!res) {
			qWarning("%s: %d: Invalid radius", qPrintable(path), ln);
			continue;
		}
		double flattening = list[3].trimmed().toDouble(&res);
		if (!res) {
			qWarning("%s: %d: Invalid flattening", qPrintable(path), ln);
			continue;
		}

		Ellipsoid e(radius, 1.0/flattening);
		_ellipsoids.insert(id, e);
	}
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
