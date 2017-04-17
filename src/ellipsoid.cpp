#include <QFile>
#include "ellipsoid.h"

QMap<int, Ellipsoid> Ellipsoid::_ellipsoids;
QString Ellipsoid::_errorString;
int Ellipsoid::_errorLine = 0;

Ellipsoid Ellipsoid::ellipsoid(int id)
{
	QMap<int, Ellipsoid>::const_iterator it = _ellipsoids.find(id);

	if (it == _ellipsoids.end())
		return Ellipsoid();

	return it.value();
}

bool Ellipsoid::loadList(const QString &path)
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
		if (list.size() != 4) {
			_errorString = "Format error";
			return false;
		}

		int id = list[0].trimmed().toInt(&res);
		if (!res) {
			_errorString = "Invalid ellipsoid id";
			return false;
		}
		double radius = list[2].trimmed().toDouble(&res);
		if (!res) {
			_errorString = "Invalid ellipsoid radius";
			return false;
		}
		double flattening = list[3].trimmed().toDouble(&res);
		if (!res) {
			_errorString = "Invalid ellipsoid flattening";
			return false;
		}

		Ellipsoid e(radius, 1.0/flattening);
		_ellipsoids.insert(id, e);

		_errorLine++;
	}

	return true;
}
