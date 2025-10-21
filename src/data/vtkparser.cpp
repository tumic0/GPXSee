#include <QtEndian>
#include <QTimeZone>
#include "common/protobuf.h"
#include "vtkparser.h"

using namespace Protobuf;

static bool trackpoint(CTX &ctx, Trackpoint &t)
{
	qint32 lon = 0xFFFFFFF, lat = 0xFFFFFFF;
	quint32 len, val, seconds = 0, centiSeconds = 0, speed = 0;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (field(ctx.tag)) {
			case 1:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, seconds))
					return false;
				break;
			case 2:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, centiSeconds))
					return false;
				break;
			case 3:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, val))
					return false;
				lat = zigzag32decode(val);
				break;
			case 4:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, val))
					return false;
				lon = zigzag32decode(val);
				break;
			case 5:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, speed))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	t.setCoordinates(Coordinates(lon / 1e7, lat / 1e7));
	t.setTimestamp(QDateTime::fromMSecsSinceEpoch(
	  ((qint64)seconds * 1000) + ((qint64)centiSeconds * 10),
	  QTimeZone::utc()));
	t.setSpeed(speed * 0.051444);

	return (ctx.bp == ee);
}

static bool record(CTX &ctx, Trackpoint &t)
{
	while (ctx.bp < ctx.be) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (field(ctx.tag)) {
			case 1:
				if (!trackpoint(ctx, t))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ctx.be);
}

bool VTKParser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	Q_UNUSED(waypoints);
	qint64 len;
	quint16 recordLen;
	QByteArray ba;
	SegmentData segment;
	Trackpoint t;

	_errorString = "";

	while (true) {
		if ((len = file->read((char*)&recordLen, sizeof(recordLen)))
		  != sizeof(recordLen)) {
			if (!len)
				break;
			else {
				_errorString = "Error reading VTK record size";
				return false;
			}
		}

		recordLen = qFromLittleEndian(recordLen);
		ba.resize(recordLen);
		if (file->read(ba.data(), ba.size()) != ba.size()) {
			_errorString = "Error reading VTK record";
			return false;
		}

		CTX ctx(ba);
		t.setCoordinates(Coordinates());
		if (!record(ctx, t)) {
			_errorString = "Invalid VTK record";
			return false;
		} else {
			if (t.coordinates().isValid())
				segment.append(t);
			else if (!t.coordinates().isNull()) {
				_errorString = "Invalid VTK record coordinates";
				return false;
			}
		}
	}

	tracks.append(segment);
	tracks.last().setFile(file->fileName());

	return true;
}
