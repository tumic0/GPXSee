#include <QRegularExpression>
#include "srtparser.h"

#define TIMESTAMP "[0-9]{2}:[0-9]{2}:[0-9]{2},[0-9]{3}"
#define FLT "[\\+-]?[0-9]+.[0-9]+"
#define INT "[\\+-]?[0-9]+"
#define DATE "[0-9]{4}\\.[0-9]{2}\\.[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}"

static bool parseDataOld(const QByteArray line, Trackpoint &tp)
{
	static const QRegularExpression gps("GPS\\((" FLT "),(" FLT ")(," INT ")?\\)");
	static const QRegularExpression rtk("RTK \\((" FLT "), (" FLT ")(, " INT ")?\\)");
	static const QRegularExpression home("HOME\\(" FLT "," FLT "\\) (" DATE ")");

	QRegularExpressionMatch mHome(home.match(line));
	if (mHome.hasMatch()) {
		tp.setTimestamp(QDateTime::fromString(mHome.captured(1),
		  "yyyy.MM.dd HH:mm:ss"));
		return true;
	}

	QRegularExpressionMatch mGps(gps.match(line));
	if (mGps.hasMatch()) {
		tp.setCoordinates(Coordinates(mGps.captured(1).toDouble(),
		  mGps.captured(2).toDouble()));
		if (mGps.capturedLength(3))
			tp.setElevation(mGps.captured(3).mid(1).toDouble());
		return true;
	}

	QRegularExpressionMatch mRtk(rtk.match(line));
	if (mRtk.hasMatch()) {
		tp.setCoordinates(Coordinates(mRtk.captured(1).toDouble(),
		  mRtk.captured(2).toDouble()));
		if (mRtk.capturedLength(3))
			tp.setElevation(mRtk.captured(3).mid(1).toDouble());
		return true;
	}

	return false;
}

static bool parseDataNew(const QByteArray line, Trackpoint &tp)
{
	static const QRegularExpression date(
	  "[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}.[0-9]+");
	static const QRegularExpression lon("\\[longitude: (" FLT ")\\]");
	static const QRegularExpression lat("\\[latitude: (" FLT ")\\]");
	static const QRegularExpression alt("\\[.*abs_alt: (" FLT ")\\.*]");

	QRegularExpressionMatch mDate(date.match(line));
	if (mDate.hasMatch()) {
		tp.setTimestamp(QDateTime::fromString(mDate.captured(0),
		  Qt::ISODateWithMs));
		return true;
	}

	QRegularExpressionMatch mLon(lon.match(line));
	QRegularExpressionMatch mLat(lat.match(line));
	if (mLon.hasMatch() && mLat.hasMatch()) {
		tp.setCoordinates(Coordinates(mLon.captured(1).toDouble(),
		  mLat.captured(1).toDouble()));

		QRegularExpressionMatch mAlt(alt.match(line));
		if (mAlt.hasMatch())
			tp.setElevation(mAlt.captured(1).toDouble());

		return true;
	}

	return false;
}

bool SRTParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons,
  QVector<Waypoint> &waypoints)
{
	Q_UNUSED(waypoints);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	static const QRegularExpression re(TIMESTAMP " --> " TIMESTAMP);
	SegmentData segment;
	int state = 0, format = -1;
	unsigned id = 0;
	Trackpoint tp;

	_errorLine = 1;
	_errorString.clear();

	while (!file->atEnd()) {
		QByteArray line = file->readLine(4096);

		switch (state) {
			case 0: {
				bool ok;
				uint sid = line.toUInt(&ok);
				if (!ok || sid != ++id) {
					_errorString = "Invalid subtitle ID";
					return false;
				}}
				state = 1;
				break;
			case 1: {
				QRegularExpressionMatch match = re.match(line);
				if (!match.hasMatch()) {
					_errorString = "Invalid subtitle time interval";
					return false;
				}}
				state = 2;
				tp = Trackpoint();
				break;
			case 2:
				if (line == "\n") {
					if (tp.coordinates().isValid()
					  && (format == 0 || !segment.size() || tp.coordinates()
					  != segment.last().coordinates()))
						segment.append(tp);
					state = 0;
				} else {
					switch (format) {
						case 0:
							parseDataOld(line, tp);
							break;
						case 1:
							parseDataNew(line, tp);
							break;
						default:
							if (parseDataOld(line, tp))
								format = 0;
							else if (parseDataNew(line, tp))
								format = 1;
					}
				}
		}

		_errorLine++;
	}

	if (segment.isEmpty()) {
		_errorString = "No GPS data found in SRT file";
		return false;
	} else {
		TrackData track(segment);
		track.setFile(file->fileName());
		tracks.append(track);
		return true;
	}
}
