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


bool CUPParser::waypoint(const QStringList &entry, QVector<Waypoint> &waypoints,
  QMap<QString, Coordinates> &turnpoints)
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

	turnpoints.insert(wp.name(), wp.coordinates());

	return true;
}

bool CUPParser::task(const QStringList &entry, QList<RouteData> &routes,
  const QMap<QString, Coordinates> &turnpoints)
{
	if (entry.size() < 2) {
		_errorString = "Invalid number of fields";
		return false;
	}

	RouteData r;
	r.setName(entry.at(0));
	for (int i = 1; i < entry.size(); i++) {
		if (!turnpoints.contains(entry.at(i))) {
			_errorString = entry.at(i) + ": unknown turnpoint";
			return false;
		}

		Waypoint w(turnpoints[entry.at(i)]);
		w.setName(entry.at(i));
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
	SegmentType st = Header;
	QStringList entry;
	QMap<QString, Coordinates> turnpoints;


	while (!csv.atEnd()) {
		if (!csv.readEntry(entry)) {
			_errorString = "CSV parse error";
			_errorLine = csv.line();
			return false;
		}

		if (st == Header) {
			st = Waypoints;
			if (entry.size() >= 11 && entry.at(3) == "lat"
			  && entry.at(4) == "lon") {
				entry.clear();
				continue;
			}
		} else if (st == Waypoints && entry.size() == 1
		  && entry.at(0) == "-----Related Tasks-----") {
			st = Tasks;
			entry.clear();
			continue;
		}

		if (st == Waypoints) {
			if (!waypoint(entry, waypoints, turnpoints))
				return false;
		} else if (st == Tasks) {
			if (entry.at(0) != "Options" && entry.at(0) != "ObsZone"
			  && !task(entry, routes, turnpoints))
				return false;
		}

		entry.clear();
		_errorLine = csv.line();
	}

	return true;
}
