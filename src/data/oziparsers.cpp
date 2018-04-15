#include "map/gcs.h"
#include "oziparsers.h"

static qint64 delphi2unixMS(double date)
{
	return (qint64)((date - 25569.0) * 86400000);
}

bool PLTParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	Q_UNUSED(routes);
	bool res;
	const GCS *gcs = 0;

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
				QByteArray field(list.at(3).trimmed());
				if (!field.isEmpty()) {
					double elevation = field.toDouble(&res);
					if (!res) {
						_errorString = "Invalid elevation";
						return false;
					}
					if (elevation != -777)
						tp.setElevation(elevation * 0.3048);
				}
			}
			if (list.size() >= 5) {
				QByteArray field(list.at(4).trimmed());
				if (!field.isEmpty()) {
					double date = field.toDouble(&res);
					if (!res) {
						_errorString = "Invalid date";
						return false;
					}
					tp.setTimestamp(QDateTime::fromMSecsSinceEpoch(
					  delphi2unixMS(date)));
				}
			}

			track.append(tp);
		}

		_errorLine++;
	}

	return true;
}

bool RTEParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	Q_UNUSED(tracks);
	bool res, record = false;
	const GCS *gcs = 0;

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
					QByteArray field(list.at(7).trimmed());
					if (!field.isEmpty()) {
						double date = field.toDouble(&res);
						if (!res) {
							_errorString = "Invalid date";
							return false;
						}
						wp.setTimestamp(QDateTime::fromMSecsSinceEpoch(
						  delphi2unixMS(date)));
					}
				}
				if (list.size() >= 14) {
					QString desc(list.at(13).trimmed().replace('\xD1', ','));
					if (!desc.isEmpty())
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

bool WPTParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	bool res;
	const GCS *gcs = 0;

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
				QByteArray field(list.at(4).trimmed());
				if (!field.isEmpty()) {
					double date = field.toDouble(&res);
					if (!res) {
						_errorString = "Invalid date";
						return false;
					}
					wp.setTimestamp(QDateTime::fromMSecsSinceEpoch(
					  delphi2unixMS(date)));
				}
			}
			if (list.size() >= 11) {
				QString desc(list.at(10).trimmed().replace('\xD1', ','));
				if (!desc.isEmpty())
					wp.setDescription(desc);
			}
			if (list.size() >= 15) {
				QByteArray field(list.at(14).trimmed());
				if (!field.isEmpty()) {
					double elevation = list.at(14).trimmed().toDouble(&res);
					if (!res) {
						_errorString = "Invalid elevation";
						return false;
					}
					if (elevation != -777)
						wp.setElevation(elevation * 0.3048);
				}
			}

			waypoints.append(wp);
		}

		_errorLine++;
	}

	return true;
}
