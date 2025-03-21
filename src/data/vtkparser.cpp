#include <QtEndian>
#include <QTimeZone>
#include "vtkparser.h"

#define TYPE(tag) (tag & 0x07)
#define FIELD(tag) (tag >> 3)

#define VARINT 0
#define I64    1
#define LEN    2
#define I32    5

struct CTX
{
	CTX(const QByteArray &ba)
	  : bp(ba.constData()), be(bp + ba.size()), tag(0) {}

	const char *bp;
	const char *be;
	quint32 tag;
};

static inline qint32 zigzag32decode(quint32 value)
{
	return static_cast<qint32>((value >> 1u) ^ static_cast<quint32>(
	  -static_cast<qint32>(value & 1u)));
}

template<typename T>
static bool varint(CTX &ctx, T &val)
{
	unsigned int shift = 0;
	val = 0;

	while (ctx.bp < ctx.be) {
		val |= ((quint8)*ctx.bp & 0x7F) << shift;
		shift += 7;
		if (!((quint8)*ctx.bp++ & 0x80))
			return true;
	}

	return false;
}

static bool length(CTX &ctx, qint32 &val)
{
	if (TYPE(ctx.tag) != LEN)
		return false;

	if (!varint(ctx, val))
		return false;

	return (val >= 0);
}

static bool skip(CTX &ctx)
{
	qint32 len = 0;

	switch (TYPE(ctx.tag)) {
		case VARINT:
			return varint(ctx, len);
		case I64:
			len = 8;
			break;
		case LEN:
			if (!varint(ctx, len) || len < 0)
				return false;
			break;
		case I32:
			len = 4;
			break;
		default:
			return false;
	}

	if (ctx.bp + len > ctx.be)
		return false;
	ctx.bp += len;

	return true;
}

static bool trackpoint(CTX &ctx, Trackpoint &t)
{
	qint32 len, lon = 0xFFFFFFF, lat = 0xFFFFFFF;
	quint32 val, seconds = 0, centiSeconds = 0, speed = 0;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (FIELD(ctx.tag)) {
			case 1:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, seconds))
					return false;
				break;
			case 2:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, centiSeconds))
					return false;
				break;
			case 3:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, val))
					return false;
				lat = zigzag32decode(val);
				break;
			case 4:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, val))
					return false;
				lon = zigzag32decode(val);
				break;
			case 5:
				if (TYPE(ctx.tag) != VARINT)
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

		switch (FIELD(ctx.tag)) {
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
	return true;
}
