#include <QFile>
#include "wgs84.h"
#include "datum.h"


static QMap<QString, Datum> WGS84()
{
	QMap<QString, Datum> map;
	map.insert("WGS 84", Datum(Ellipsoid(WGS84_RADIUS, WGS84_FLATTENING),
	  0, 0, 0));
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
