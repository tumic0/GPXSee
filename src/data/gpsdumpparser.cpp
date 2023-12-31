#include <QRegularExpression>
#include "map/pcs.h"
#include "map/gcs.h"
#include "map/utm.h"
#include "gpsdumpparser.h"

static double dms2dd(const QStringList &dms)
{
	bool ok;

	int deg = dms.at(1).toInt(&ok);
	if (!ok)
		return NAN;
	int min = dms.at(2).toInt(&ok);
	if (!ok)
		return NAN;
	double sec = dms.at(3).toDouble(&ok);
	if (!ok)
		return NAN;

	return deg + min/60.0 + sec/3600.0;
}

static double parseLon(const QString &str)
{
	QStringList dms(str.split(' '));
	if (dms.size() < 4)
		return NAN;

	double dd = dms2dd(dms);
	if (std::isnan(dd))
		return NAN;

	if (dms.at(0) == 'W')
		return -dd;
	else if (dms.at(0) == 'E')
		return dd;
	else
		return NAN;
}

static double parseLat(const QString &str)
{
	QStringList dms(str.split(' '));
	if (dms.size() < 4)
		return NAN;

	double dd = dms2dd(dms);
	if (std::isnan(dd))
		return NAN;

	if (dms.at(0) == 'S')
		return -dd;
	else if (dms.at(0) == 'N')
		return dd;
	else
		return NAN;
}

static Coordinates parseGEO(const QString &lat, const QString &lon)
{
	return Coordinates(parseLon(lon), parseLat(lat));
}

static Coordinates parseUTM(const QString &zone, const QString &easting,
  const QString &northing)
{
	bool ok;

	int z = zone.left(zone.size() - 1).toInt(&ok);
	if (!ok)
		return Coordinates();
	if (zone.right(1) < 'N')
		z = -z;

	int x = easting.toInt(&ok);
	if (!ok)
		return Coordinates();
	int y = northing.toInt(&ok);
	if (!ok)
		return Coordinates();

	Projection proj(PCS(GCS::WGS84(), Conversion(9807, UTM::setup(z), 9001)));

	return proj.xy2ll(PointD(x, y));
}

bool GPSDumpParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	static const QRegularExpression dm("[ ]{2,}");
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);

	_errorLine = 1;
	_errorString.clear();
	Type type = Unknown;

	while (!file->atEnd()) {
		QByteArray ba(file->readLine(4096).trimmed());

		if (_errorLine == 1) {
			if (ba == "$FormatGEO")
				type = GEO;
			else if (ba == "$FormatUTM")
				type = UTM;
			else {
				_errorString = "Not a GPSDump waypoint file";
				return false;
			}
		} else if (!ba.isEmpty()) {
			QString line(ba);
			QStringList fields(line.split(dm));
			Coordinates c;
			double ele = NAN;
			QString desc;
			bool ok;

			if (type == UTM) {
				if (fields.size() < 5) {
					_errorString = "Parse error";
					return false;
				}

				c = parseUTM(fields.at(1), fields.at(2), fields.at(3));
				ele = fields.at(4).toDouble(&ok);
				if (fields.size() > 5)
					desc = fields.at(5);
			} else {
				if (fields.size() < 4) {
					_errorString = "Parse error";
					return false;
				}

				c = parseGEO(fields.at(1), fields.at(2));
				ele = fields.at(3).toDouble(&ok);
				if (fields.size() > 4)
					desc = fields.at(4);
			}

			if (!c.isValid()) {
				_errorString = "Invalid coordinates";
				return false;
			}

			Waypoint w(c);
			w.setName(fields.at(0));
			if (ok)
				w.setElevation(ele);
			if (!desc.isEmpty())
				w.setDescription(desc);

			waypoints.append(w);
		}

		_errorLine++;
	}

	return true;
}
