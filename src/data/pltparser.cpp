#include "map/gcs.h"
#include "date.h"
#include "pltparser.h"


bool PLTParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	Q_UNUSED(routes);
	bool res;
	const GCS *gcs;

	_errorLine = 1;
	_errorString.clear();

	tracks.append(TrackData());
	TrackData &track = tracks.last();

	while (!file->atEnd()) {
		QByteArray line = file->readLine();

		if (_errorLine == 1) {
			if (!line.trimmed().startsWith("OziExplorer Track Point File")) {
				_errorString = "Not a PLT file";
				return false;
			}
		} else if (_errorLine == 2) {
			if (!(gcs = GCS::gcs(QString(line.trimmed())))) {
				_errorString = "Invalid/unknown datum";
				return false;
			}
		} else if (_errorLine == 5) {
			QList<QByteArray> list = line.split(',');
			if (list.size() >= 4)
				track.setName(list.at(3));
		} else if (_errorLine > 6) {
			QList<QByteArray> list = line.split(',');
			if (list.size() < 2) {
				_errorString = "Parse error";
				return false;
			}

			qreal lat = list.at(0).trimmed().toDouble(&res);
			if (!res || (lat < -90.0 || lat > 90.0)) {
				_errorString = "Invalid latitude";
				return false;
			}
			qreal lon = list.at(1).trimmed().toDouble(&res);
			if (!res || (lon < -180.0 || lon > 180.0)) {
				_errorString = "Invalid longitude";
				return false;
			}

			Trackpoint tp(gcs->toWGS84(Coordinates(lon, lat)));

			if (list.size() >= 4) {
				double elevation = list.at(3).trimmed().toDouble(&res);
				if (!res) {
					_errorString = "Invalid elevation";
					return false;
				}
				if (elevation != -777)
					tp.setElevation(elevation * 0.3048);
			}
			if (list.size() >= 5) {
				double date = list.at(4).trimmed().toDouble(&res);
				if (!res) {
					_errorString = "Invalid date";
					return false;
				}
				tp.setTimestamp(QDateTime::fromMSecsSinceEpoch(
				  Date::delphi2unixMS(date)));
			}

			track.append(tp);
		}

		_errorLine++;
	}

	return true;
}
