#include <cmath>
#include <QStringList>
#include "csv.h"
#include "cupparser.h"


enum SegmentType {
	Header, Waypoints, Tasks
};

static double latitude(const QString &str)
{
	bool ok;

	if (str.length() != 9)
		return NAN;
	int deg = str.left(2).toInt(&ok);
	if (!ok || deg > 90)
		return NAN;
	double min = str.mid(2, 6).toDouble(&ok);
	if (!ok || min > 60)
		return NAN;

	double dd = deg + min/60.0;
	return (str.right(1) == "S") ? -dd : dd;
}

static double longitude(const QString &str)
{
	bool ok;

	if (str.length() != 10)
		return NAN;
	int deg = str.left(3).toInt(&ok);
	if (!ok || deg > 180)
		return NAN;
	double min = str.mid(3, 6).toDouble(&ok);
	if (!ok || min > 60)
		return NAN;

	double dd = deg + min/60.0;
	return (str.right(1) == "W") ? -dd : dd;
}

static double elevation(const QString &str)
{
	bool ok;
	double ele;

	if (str.right(2) == "ft")
		ele = str.left(str.length() - 2).toDouble(&ok) * 0.3048;
	else if (str.right(1) == "m")
		ele = str.left(str.length() - 1).toDouble(&ok);
	else
		return NAN;

	return ok ? ele : NAN;
}


bool CUPParser::waypoint(const QStringList &entry, QVector<Waypoint> &waypoints)
{
	if (entry.size() < 11) {
		_errorString = "Invalid number of fields";
		return false;
	}

	double lon = longitude(entry.at(4));
	if (std::isnan(lon)) {
		_errorString = "Invalid longitude";
		return false;
	}
	double lat = latitude(entry.at(3));
	if (std::isnan(lat)) {
		_errorString = "Invalid latitude";
		return false;
	}

	Waypoint wp(Coordinates(lon, lat));
	wp.setName(entry.at(0));
	wp.setDescription(entry.at(10));
	wp.setElevation(elevation(entry.at(5)));
	waypoints.append(wp);

	return true;
}

bool CUPParser::task(const QStringList &entry,
  const QVector<Waypoint> &waypoints, QList<RouteData> &routes)
{
	if (entry.size() < 3) {
		_errorString = "Invalid number of fields";
		return false;
	}

	RouteData r;
	r.setName(entry.at(0));
	for (int i = 1; i < entry.size(); i++) {
		if (entry.at(i) == "???")
			continue;

		Waypoint w;
		for (int j = 0; j < waypoints.size(); j++) {
			if (waypoints.at(j).name() == entry.at(i)) {
				w = waypoints.at(j);
				break;
			}
		}
		if (w.coordinates().isNull()) {
			_errorString = entry.at(i) + ": unknown turnpoint";
			return false;
		}
		r.append(w);
	}

	routes.append(r);

	return true;
}

bool CUPParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(polygons);
	CSV csv(file);
	QStringList entry;
	SegmentType segment = Header;


	while (!csv.atEnd()) {
		if (!csv.readEntry(entry)) {
			_errorString = "CSV parse error";
			_errorLine = csv.line();
			return false;
		}

		if (segment == Header) {
			segment = Waypoints;
			if (entry.size() >= 11 && entry.at(3) == "lat"
			  && entry.at(4) == "lon") {
				entry.clear();
				continue;
			}
		} else if (segment == Waypoints && entry.size() == 1
		  && entry.at(0) == "-----Related Tasks-----") {
			segment = Tasks;
			entry.clear();
			continue;
		}

		if (segment == Waypoints) {
			if (!waypoint(entry, waypoints))
				return false;
		} else if (segment == Tasks) {
			if (entry.at(0) != "Options" && !entry.at(0).startsWith("ObsZone=")
			  && !task(entry, waypoints, routes))
				return false;
		}

		entry.clear();
		_errorLine = csv.line();
	}

	return true;
}
