#include "csvparser.h"

bool CSVParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	bool res;

	_errorLine = 1;
	_errorString.clear();

	while (!file->atEnd()) {
		QByteArray line = file->readLine();
		QList<QByteArray> list = line.split(',');
		if (list.size() < 3) {
			_errorString = "Parse error";
			return false;
		}

		qreal lon = list[0].trimmed().toDouble(&res);
		if (!res || (lon < -180.0 || lon > 180.0)) {
			_errorString = "Invalid longitude";
			return false;
		}
		qreal lat = list[1].trimmed().toDouble(&res);
		if (!res || (lat < -90.0 || lat > 90.0)) {
			_errorString = "Invalid latitude";
			return false;
		}
		Waypoint wp(Coordinates(lon, lat));

		QByteArray ba = list[2].trimmed();
		QString name = QString::fromUtf8(ba.data(), ba.size());
		wp.setName(name);

		if (list.size() > 3) {
			ba = list[3].trimmed();
			wp.setDescription(QString::fromUtf8(ba.data(), ba.size()));
		}

		waypoints.append(wp);
		_errorLine++;
	}

	return true;
}
