#include <QByteArray>
#include "common/textcodec.h"
#include "itnparser.h"

bool ITNParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(waypoints);
	Q_UNUSED(polygons);
	RouteData rd;
	QByteArray ba;
	TextCodec codec(1252);
	int lat, lon;
	bool ok1, ok2;

	_errorLine = 1;
	_errorString.clear();

	while (!file->atEnd()) {
		ba = file->readLine();

		QList<QByteArray> fields(ba.split('|'));
		if (fields.size() < 4) {
			_errorString = "File format error";
			return false;
		}

		lon = fields.at(0).toInt(&ok1);
		lat = fields.at(1).toInt(&ok2);
		if (!ok1 || !ok2 || lon < -18000000 || lon > 18000000
		  || lat < -9000000 || lat > 9000000) {
			_errorString = "Invalid coordinates";
			return false;
		}
		Waypoint wp(Coordinates(lon/1e5, lat/1e5));
		wp.setName(codec.toString(fields.at(2)));
		rd.append(wp);

		_errorLine++;
	}

	routes.append(rd);

	return true;
}
