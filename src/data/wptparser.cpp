#include "map/gcs.h"
#include "date.h"
#include "wptparser.h"


bool WPTParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	bool res;
	const GCS *gcs;

	_errorLine = 1;
	_errorString.clear();

	while (!file->atEnd()) {
		QByteArray line = file->readLine();

		if (_errorLine == 1) {
			if (!line.trimmed().startsWith("OziExplorer Waypoint File")) {
				_errorString = "Not a WPT file";
				return false;
			}
		} else if (_errorLine == 2) {
			if (!(gcs = GCS::gcs(QString(line.trimmed())))) {
				_errorString = "Invalid/unknown datum";
				return false;
			}
		} else if (_errorLine > 4) {
			QList<QByteArray> list = line.split(',');
			if (list.size() < 4) {
				_errorString = "Parse error";
				return false;
			}

			qreal lat = list.at(2).trimmed().toDouble(&res);
			if (!res || (lat < -90.0 || lat > 90.0)) {
				_errorString = "Invalid latitude";
				return false;
			}
			qreal lon = list.at(3).trimmed().toDouble(&res);
			if (!res || (lon < -180.0 || lon > 180.0)) {
				_errorString = "Invalid longitude";
				return false;
			}

			Waypoint wp(gcs->toWGS84(Coordinates(lon, lat)));

			QString name(list.at(1).trimmed().replace('\xD1', ','));
			if (!name.isEmpty())
				wp.setName(name);
			if (list.size() >= 5) {
				double date = list.at(4).trimmed().toDouble(&res);
				if (!res) {
					_errorString = "Invalid date";
					return false;
				}
				wp.setTimestamp(QDateTime::fromMSecsSinceEpoch(
				  Date::delphi2unixMS(date)));
			}
			if (list.size() >= 11) {
				QString desc(list.at(10).trimmed().replace('\xD1', ','));
				if (!desc.isEmpty())
					wp.setDescription(desc);
			} if (list.size() >= 15) {
				double elevation = list.at(14).trimmed().toDouble(&res);
				if (!res) {
					_errorString = "Invalid elevation";
					return false;
				}
				if (elevation != -777)
					wp.setElevation(elevation * 0.3048);
			}

			waypoints.append(wp);
		}

		_errorLine++;
	}

	return true;
}
