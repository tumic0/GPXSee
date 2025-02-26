#include <QTimeZone>
#include "common/csv.h"
#include "txtparser.h"

static Coordinates coordinates(const QByteArrayList &entry)
{
	bool lonOk, latOk;
	double lon = entry.at(3).toDouble(&lonOk);
	double lat = entry.at(2).toDouble(&latOk);

	return (lonOk && latOk) ? Coordinates(lon, lat) : Coordinates();
}

bool TXTParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	Q_UNUSED(waypoints);
	CSV csv(file);
	QByteArrayList entry;
	SegmentData *sg = 0;

	_errorLine = 1;
	_errorString.clear();

	while (!csv.atEnd()) {
		if (!csv.readEntry(entry)) {
			_errorString = "CSV parse error";
			_errorLine = csv.line() - 1;
			return false;
		}

		if (entry.size() == 1) {
			if (entry.at(0) == "$V02") {
				tracks.append(TrackData(SegmentData()));
				sg = &tracks.last().last();
			} else {
				_errorString = "Invalid track start marker";
				_errorLine = csv.line() - 1;
				return false;
			}
		} else {
			if (!sg) {
				_errorString = "Missing start marker";
				_errorLine = csv.line() - 1;
				return false;
			}

			if (entry.size() == 13 && entry.at(1) == "A") {
				Coordinates c(coordinates(entry));
				if (!c.isValid()) {
					_errorString = "Invalid coordinates";
					_errorLine = csv.line() - 1;
					return false;
				}
				Trackpoint tp(c);

				bool ok;
				qulonglong ts = entry.at(0).toULongLong(&ok);
				if (!ok) {
					_errorString = "Invalid timestamp";
					_errorLine = csv.line() - 1;
					return false;
				}
				tp.setTimestamp(QDateTime::fromSecsSinceEpoch(ts,
				  QTimeZone::utc()));

				uint speed = entry.at(5).toULong(&ok);
				if (ok)
					tp.setSpeed(speed * 0.01);

				if (c != Coordinates(0, 0))
					sg->append(tp);
			}
		}
	}

	return true;
}
