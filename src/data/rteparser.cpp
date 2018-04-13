#include "map/gcs.h"
#include "date.h"
#include "rteparser.h"


bool RTEParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	Q_UNUSED(tracks);
	bool res, record = false;
	const GCS *gcs;

	_errorLine = 1;
	_errorString.clear();


	while (!file->atEnd()) {
		QByteArray line = file->readLine();

		if (_errorLine == 1) {
			if (!line.trimmed().startsWith("OziExplorer Route File")) {
				_errorString = "Not a RTE file";
				return false;
			}
		} else if (_errorLine == 2) {
			if (!(gcs = GCS::gcs(QString(line.trimmed())))) {
				_errorString = "Invalid/unknown datum";
				return false;
			}
		} else if (_errorLine > 4) {
			QList<QByteArray> list = line.split(',');
			if (list.size() < 2) {
				_errorString = "Parse error";
				return false;
			}

			if (list.at(0).trimmed() == "R") {
				routes.append(RouteData());
				record = true;

				if (list.size() >= 3)
					routes.last().setName(list.at(2).trimmed()
					  .replace('\xD1', ','));
				if (list.size() >= 4)
					routes.last().setDescription(list.at(3).trimmed()
					  .replace('\xD1', ','));
			} else if (list.at(0).trimmed() == "W") {
				if (!record || list.size() < 7) {
					_errorString = "Parse error";
					return false;
				}

				qreal lat = list.at(5).trimmed().toDouble(&res);
				if (!res || (lat < -90.0 || lat > 90.0)) {
					_errorString = "Invalid latitude";
					return false;
				}
				qreal lon = list.at(6).trimmed().toDouble(&res);
				if (!res || (lon < -180.0 || lon > 180.0)) {
					_errorString = "Invalid longitude";
					return false;
				}

				Waypoint wp(gcs->toWGS84(Coordinates(lon, lat)));

				QString name(list.at(4).trimmed().replace('\xD1', ','));
				if (!name.isEmpty())
					wp.setName(name);
				if (list.size() >= 8) {
					double date = list.at(7).trimmed().toDouble(&res);
					if (!res) {
						_errorString = "Invalid date";
						return false;
					}
					wp.setTimestamp(QDateTime::fromMSecsSinceEpoch(
					  Date::delphi2unixMS(date)));
				}
				if (list.size() >= 14) {
					QString desc(list.at(13).trimmed().replace('\xD1', ','));
					wp.setDescription(desc);
				}

				routes.last().append(wp);
			} else {
				_errorString = "Parse error";
				return false;
			}
		}

		_errorLine++;
	}

	return true;
}
