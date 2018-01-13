#include <QFile>
#include <QDebug>
#include "ellipsoid.h"

QMap<int, Ellipsoid> Ellipsoid::_ellipsoids;
QString Ellipsoid::_errorString;
int Ellipsoid::_errorLine = 0;

Ellipsoid::Ellipsoid(int id)
{
	QMap<int, Ellipsoid>::const_iterator it = _ellipsoids.find(id);

	if (it == _ellipsoids.end())
		*this = Ellipsoid();
	else
		*this = it.value();
}

void Ellipsoid::error(const QString &str)
{
	_errorString = str;
	_ellipsoids.clear();
}

bool Ellipsoid::loadList(const QString &path)
{
	QFile file(path);
	bool res;


	if (!file.open(QFile::ReadOnly)) {
		error(file.errorString());
		return false;
	}

	_errorLine = 1;
	_errorString.clear();

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() != 4) {
			error("Format error");
			return false;
		}

		int id = list[1].trimmed().toInt(&res);
		if (!res) {
			error("Invalid ellipsoid id");
			return false;
		}
		double radius = list[2].trimmed().toDouble(&res);
		if (!res) {
			error("Invalid ellipsoid radius");
			return false;
		}
		double flattening = list[3].trimmed().toDouble(&res);
		if (!res) {
			error("Invalid ellipsoid flattening");
			return false;
		}

		Ellipsoid e(radius, 1.0/flattening);
		_ellipsoids.insert(id, e);

		_errorLine++;
	}

	return true;
}

QDebug operator<<(QDebug dbg, const Ellipsoid &ellipsoid)
{
	dbg.nospace() << "Ellipsoid(" << ellipsoid.radius() << ", "
	  << 1.0 / ellipsoid.flattening() << ")";
	return dbg.space();
}
