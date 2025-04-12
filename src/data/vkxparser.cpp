#include "vkxparser.h"

static bool readTrackPoint(QDataStream &stream, SegmentData &segment)
{
	quint64 time;
	qint32 lat, lon;
	quint32 unused;
	float speed, alt;

	stream >> time >> lat >> lon;
	if (stream.status() != QDataStream::Ok)
		return false;
	if (stream.readRawData((char*)&speed, 4) != 4)
		return false;
	stream >> unused;
	if (stream.readRawData((char*)&alt, 4) != 4)
		return false;
	stream >> unused >> unused >> unused >> unused;
	if (stream.status() != QDataStream::Ok)
		return false;

	Trackpoint t(Coordinates(lon / 1e7, lat / 1e7));
	if (!t.coordinates().isValid())
		return false;
	t.setTimestamp(QDateTime::fromMSecsSinceEpoch(time));
	t.setSpeed(speed);
	t.setElevation(alt);

	segment.append(t);

	return true;
}

bool VKXParser::skip(QDataStream &stream, quint8 key, int len)
{
	if (stream.skipRawData(len) != len) {
		_errorString = "Invalid 0x" + QString::number(key, 16) + " row";
		return false;
	}

	return true;
}

bool VKXParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	Q_UNUSED(waypoints);
	quint8 key;
	quint64 hdr;
	SegmentData segment;

	QDataStream stream(file);
	stream.setByteOrder(QDataStream::LittleEndian);

	stream >> hdr;
	if ((hdr & 0xFF) != 0xFF) {
		_errorString = "Not a Vakaros VKX file";
		return false;
	}

	while (stream.status() == QDataStream::Ok) {
		stream >> key;
		if (stream.status() != QDataStream::Ok)
			break;

		switch (key) {
			case 0x01:
				if (!skip(stream, key, 32))
					return false;
				break;
			case 0x02:
				if (!readTrackPoint(stream, segment)) {
					_errorString = "Invalid 0x2 row";
					return false;
				}
				break;
			case 0x03:
				if (!skip(stream, key, 20))
					return false;
				break;
			case 0x04:
				if (!skip(stream, key, 13))
					return false;
				break;
			case 0x05:
				if (!skip(stream, key, 17))
					return false;
				break;
			case 0x06:
				if (!skip(stream, key, 18))
					return false;
				break;
			case 0x07:
				if (!skip(stream, key, 12))
					return false;
				break;
			case 0x08:
				if (!skip(stream, key, 13))
					return false;
				break;
			case 0x0A:
			case 0x0B:
				if (!skip(stream, key, 16))
					return false;
				break;
			case 0x0C:
				if (!skip(stream, key, 12))
					return false;
				break;
			case 0x0E:
			case 0x0F:
				if (!skip(stream, key, 16))
					return false;
				break;
			case 0x10:
				if (!skip(stream, key, 12))
					return false;
				break;
			case 0x20:
				if (!skip(stream, key, 13))
					return false;
				break;
			case 0x21:
				if (!skip(stream, key, 52))
					return false;
				break;
			case 0xFE:
				if (!skip(stream, key, 2))
					return false;
				break;
			case 0xFF:
				if (!skip(stream, key, 7))
					return false;
				break;
			default:
				_errorString = "Unknown row key: 0x" + QString::number(key, 16);
				return false;
		}
	}

	if (stream.status() != QDataStream::ReadPastEnd) {
		_errorString = "Unexpected EOF";
		return false;
	}

	tracks.append(segment);

	return true;
}
