#include "csvparser.h"

bool CSVParser::loadFile(QIODevice *device)
{
	bool ret;
	int ln = 1;

	_errorLine = 0;
	_errorString.clear();

	while (!device->atEnd()) {
		QByteArray line = device->readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() < 3) {
			_errorString = "Parse error.";
			_errorLine = ln;
			return false;
		}

		qreal lat = list[0].trimmed().toDouble(&ret);
		if (!ret) {
			_errorString = "Invalid latitude.";
			_errorLine = ln;
			return false;
		}
		qreal lon = list[1].trimmed().toDouble(&ret);
		if (!ret) {
			_errorString = "Invalid longitude.";
			_errorLine = ln;
			return false;
		}
		Waypoint wp(QPointF(lon, lat));

		QByteArray ba = list[2].trimmed();
		QString name = QString::fromUtf8(ba.data(), ba.size());
		wp.setName(name);

		if (list.size() > 3) {
			ba = list[3].trimmed();
			wp.setDescription(QString::fromUtf8(ba.data(), ba.size()));
		}

		_waypoints.append(wp);
		ln++;
	}

	return true;
}
