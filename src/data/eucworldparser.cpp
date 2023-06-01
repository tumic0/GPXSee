#include <QByteArrayList>
#include "common/csv.h"
#include "eucworldparser.h"

bool EUCWORLDParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	CSV csv(file);
	QByteArrayList entry;
	bool ok;

	// euc.world log files start with the field names (no less than 40)
	//
	if (!csv.readEntry(entry) || entry.size() < 40 || entry.at(0) != "datetime") {
		_errorString = "Parse error (header not found)";
		_errorLine = csv.line();
		return false;
	}

	while (!csv.atEnd()) {
		if (!csv.readEntry(entry) || entry.size() < 40) {
			_errorString = "Parse error";
			_errorLine = csv.line();
			return false;
		}

		// fundamental fields:
		//	0:  device timestamp, always present
		//	5:  reported wheel speed (kph), always present
		// 	25: gps_datetime
		// 	29: gps_lat
		// 	30: gps_lon
		// 	31: gps_speed
		// 	35: gps_alt
		//
		double lon = entry.at(30).toDouble(&ok);
		if (!ok) {
			continue;
		}
		if (lon < -180.0 || lon > 180.0) {
			_errorString = "Invalid longitude";
			_errorLine = csv.line();
			return false;
		}

		double lat = entry.at(29).toDouble(&ok);
		if (!ok) {
			continue;
		}
		if (lat < -90.0 || lat > 90.0) {
			_errorString = "Invalid latitude";
			_errorLine = csv.line();
			return false;
		}
		Waypoint wp(Coordinates(lon, lat));

		double el = entry.at(35).toDouble(&ok);
		if (ok) {
			wp.setElevation(el);
		}

		if (lat != lastlat || lon != lastlon) {
			waypoints.append(wp);
		}

		lastlat = lat;
		lastlon = lon;
	}

	return true;
}
