#include "common/rectc.h"
#include "units.h"
#include "lblfile.h"
#include "netfile.h"
#include "rgnfile.h"


static int bitSize(quint8 baseSize, bool variableSign, bool extraBit)
{
	int bits = 2;
	if (baseSize <= 9)
		bits += baseSize;
	else
		bits += 2 * baseSize - 9;

	if (variableSign)
		bits++;
	if (extraBit)
		bits++;

	return bits;
}

bool RGNFile::BitStream::read(int bits, quint32 &val)
{
	val = 0;

	for (int pos = 0; pos < bits; ) {
		if (!_remaining) {
			if (!_length || !_file.readByte(_hdl, _data))
				return false;
			_remaining = 8;
			_length--;
		}

		quint32 get = bits - pos;
		if (get >= _remaining) {
			val |= _data << pos;
			pos += _remaining;
			_remaining = 0;
		} else {
			quint32 mask = (1<<get) - 1;
			val |= (_data & mask)<<pos;
			_data >>= get;
			_remaining -= get;
			break;
		}
	}

	return true;
}

bool RGNFile::BitStream::flush()
{
	while (_length--)
		if (!_file.readByte(_hdl, _data))
			return false;
	return true;
}

RGNFile::DeltaStream::DeltaStream(const SubFile &file, Handle &hdl,
  quint32 length, quint8 info, bool extraBit, bool extended)
  : BitStream(file, hdl, length), _lonBits(0), _latBits(0)
{
	_extraBit = extraBit ? 1 : 0;
	if (!(sign(_lonSign) && sign(_latSign)))
		return;
	if (extended) {
		quint32 b;
		if (!read(1, b))
			return;
	}
	_lonBits = bitSize(info & 0x0F, !_lonSign, extraBit);
	_latBits = bitSize(info >> 4, !_latSign, false);
}

bool RGNFile::DeltaStream::readDelta(int bits, int sign, int extraBit,
  qint32 &delta)
{
	quint32 value;

	if (!read(bits, value))
		return false;

	value >>= extraBit;

	if (!sign) {
		qint32 signMask = 1 << (bits - extraBit - 1);
		if (value & signMask) {
			qint32 comp = value ^ signMask;
			if (comp)
				delta = comp - signMask;
			else {
				qint32 other;
				if (!readDelta(bits - extraBit, sign, false, other))
					return false;
				if (other < 0)
					delta = 1 - signMask + other;
				else
					delta = signMask - 1 + other;
			}
		} else {
			delta = value;
		}
	} else {
		delta = value * sign;
	}

	return true;
}

bool RGNFile::DeltaStream::sign(int &val)
{
	quint32 bit;
	val = 0;

	if (!read(1, bit))
		return false;
	if (bit) {
		if (!read(1, bit))
			return false;
		val = bit ? -1 : 1;
	}

	return true;
}


bool RGNFile::init()
{
	Handle hdl;
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size)))
		return false;

	if (hdrLen >= 0x5D) {
		if (!(readUInt32(hdl, _polygonsOffset) && readUInt32(hdl, _polygonsSize)
		  && seek(hdl, _gmpOffset + 0x39) && readUInt32(hdl, _linesOffset)
		  && readUInt32(hdl, _linesSize) && seek(hdl, _gmpOffset + 0x55)
		  && readUInt32(hdl, _pointsOffset) && readUInt32(hdl, _pointsSize)))
			return false;
	}

	if (hdrLen >= 0x7D) {
		quint32 dictOffset, dictSize;
		if (!(seek(hdl, _gmpOffset + 0x71) && readUInt32(hdl, dictOffset)
		  && readUInt32(hdl, dictSize)))
			return false;

		// NT maps
		if (dictSize || dictOffset)
			return false;
	}

	_init = true;

	return true;
}

bool RGNFile::polyObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
  const Segment &segment, LBLFile *lbl, Handle &lblHdl, NETFile *net,
  Handle &netHdl, QList<IMG::Poly> *polys) const
{
	if (segment.start() == segment.end())
		return true;

	if (!seek(hdl, segment.start()))
		return false;

	quint32 labelPtr;
	quint8 type, len8, bitstreamInfo;
	qint16 lon, lat;
	quint16 len;

	while (hdl.pos < (int)segment.end()) {
		IMG::Poly poly;

		if (!(readByte(hdl, type) && readUInt24(hdl, labelPtr)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;
		if (type & 0x80) {
			if (!readUInt16(hdl, len))
				return false;
		} else {
			if (!readByte(hdl, len8))
				return false;
			len = len8;
		}
		if (!readByte(hdl, bitstreamInfo))
			return false;

		poly.type = (segment.type() == Segment::Polygon)
		  ? ((quint32)(type & 0x7F)) << 8 : ((quint32)(type & 0x3F)) << 8;

		RectC br;
		QPoint pos(subdiv->lon() + ((qint32)lon<<(24-subdiv->bits())),
		  subdiv->lat() + ((qint32)lat<<(24-subdiv->bits())));
		Coordinates c(toWGS84(pos.x()), toWGS84(pos.y()));
		br = br.united(c);
		poly.points.append(QPointF(c.lon(), c.lat()));

		DeltaStream stream(*this, hdl, len, bitstreamInfo, labelPtr & 0x400000,
		  false);
		if (!stream.isValid())
			return false;
		while (stream.hasNext()) {
			qint32 lonDelta, latDelta;

			if (!(stream.readNext(lonDelta, latDelta)))
				return false;

			pos.rx() += lonDelta<<(24-subdiv->bits());
			pos.ry() += latDelta<<(24-subdiv->bits());

			Coordinates c(toWGS84(pos.x()), toWGS84(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			br = br.united(c);
		}
		if (!stream.flush())
			return false;

		if (!rect.intersects(br))
			continue;

		if (lbl && (labelPtr & 0x3FFFFF)) {
			if (labelPtr & 0x800000) {
				quint32 lblOff;
				if (net && net->lblOffset(netHdl, labelPtr & 0x3FFFFF, lblOff)
				  && lblOff)
					poly.label = lbl->label(lblHdl, lblOff);
			} else
				poly.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF);
		}

		polys->append(poly);
	}

	return true;
}

bool RGNFile::extPolyObjects(const RectC &rect, Handle &hdl,
  const SubDiv *subdiv, const Segment &segment, LBLFile *lbl, Handle &lblHdl,
  QList<IMG::Poly> *polys) const
{
	quint32 labelPtr, len;
	quint8 type, subtype, bitstreamInfo;
	qint16 lon, lat;


	if (!seek(hdl, segment.start()))
		return false;

	while (hdl.pos < (int)segment.end()) {
		IMG::Poly poly;

		if (!(readByte(hdl, type) && readByte(hdl, subtype)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)
		  && readVUInt32(hdl, len) && readByte(hdl, bitstreamInfo)))
			return false;

		if (subtype & 0x80) {
			qWarning("Polygons/lines with extra bytes not supported");
			return false;
		}

		poly.type = 0x10000 + (quint16(type) << 8) + (subtype & 0x1F);

		RectC br;
		QPoint pos(subdiv->lon() + ((qint32)lon<<(24-subdiv->bits())),
		  subdiv->lat() + ((qint32)lat<<(24-subdiv->bits())));
		Coordinates c(toWGS84(pos.x()), toWGS84(pos.y()));
		br = br.united(c);
		poly.points.append(QPointF(c.lon(), c.lat()));

		DeltaStream stream(*this, hdl, len - 1, bitstreamInfo, false, true);
		if (!stream.isValid())
			return false;
		while (stream.hasNext()) {
			qint32 lonDelta, latDelta;

			if (!(stream.readNext(lonDelta, latDelta)))
				return false;

			pos.rx() += lonDelta<<(24-subdiv->bits());
			pos.ry() += latDelta<<(24-subdiv->bits());

			Coordinates c(toWGS84(pos.x()), toWGS84(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			br = br.united(c);
		}
		if (!stream.flush())
			return false;

		if (subtype & 0x20) {
			if (!readUInt24(hdl, labelPtr))
				return false;
			if (lbl && (labelPtr & 0x3FFFFF))
				poly.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF);
		}

		if (!rect.intersects(br))
			continue;

		polys->append(poly);
	}

	return true;
}

bool RGNFile::pointObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
  const Segment &segment, LBLFile *lbl, Handle &lblHdl,
  QList<IMG::Point> *points) const
{
	quint8 type, subtype;
	qint16 lon, lat;
	quint32 labelPtr;

	if (!seek(hdl, segment.start()))
		return false;

	while (hdl.pos < (int)segment.end()) {
		IMG::Point point;

		if (!(readByte(hdl, type) && readUInt24(hdl, labelPtr)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;
		if (labelPtr & 0x800000) {
			if (!readByte(hdl, subtype))
				return false;
		} else
			subtype = 0;

		point.type = (quint16)type<<8 | subtype;

		qint16 lonOffset = lon<<(24-subdiv->bits());
		qint16 latOffset = lat<<(24-subdiv->bits());
		point.coordinates = Coordinates(toWGS84(subdiv->lon() + lonOffset),
		  toWGS84(subdiv->lat() + latOffset));

		if (!rect.contains(point.coordinates))
			continue;

		point.poi = labelPtr & 0x400000;
		if (lbl && (labelPtr & 0x3FFFFF)) {
			point.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF, point.poi);
			point.id = ((quint64)point.type)<<40 | ((quint64)lbl->offset())<<24
			  | (labelPtr & 0x3FFFFF);
		}

		points->append(point);
	}

	return true;
}

bool RGNFile::extPointObjects(const RectC &rect, Handle &hdl,
  const SubDiv *subdiv, const Segment &segment, LBLFile *lbl, Handle &lblHdl,
  QList<IMG::Point> *points) const
{
	quint8 type, subtype;
	qint16 lon, lat;
	quint32 labelPtr;

	if (!seek(hdl, segment.start()))
		return false;

	while (hdl.pos < (int)segment.end()) {
		IMG::Point point;

		if (!(readByte(hdl, type) && readByte(hdl, subtype)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;

		if (subtype & 0x80) {
			qWarning("Points with extra bytes not supported");
			return false;
		}

		point.type = 0x10000 | (((quint32)type)<<8) | (subtype & 0x1F);

		qint16 lonOffset = lon<<(24-subdiv->bits());
		qint16 latOffset = lat<<(24-subdiv->bits());
		point.coordinates = Coordinates(toWGS84(subdiv->lon() + lonOffset),
		  toWGS84(subdiv->lat() + latOffset));

		if (subtype & 0x20) {
			if (!readUInt24(hdl, labelPtr))
				return false;
			point.poi = labelPtr & 0x400000;
			if (lbl && (labelPtr & 0x3FFFFF)) {
				point.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF, point.poi);
				point.id = ((quint64)point.type)<<40
				  | ((quint64)lbl->offset())<<24 | (labelPtr & 0x3FFFFF);
			}
		}

		if (rect.contains(point.coordinates))
			points->append(point);
	}

	return true;
}


void RGNFile::objects(const RectC &rect, const SubDiv *subdiv, LBLFile *lbl,
  NETFile *net, QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
  QList<IMG::Point> *points)
{
	Handle rgnHdl, lblHdl, netHdl;

	if (!_init && !init())
		return;

	QVector<RGNFile::Segment> seg(segments(rgnHdl, subdiv));
	for (int i = 0; i < seg.size(); i++) {
		switch (seg.at(i).type()) {
			case Segment::Point:
			case Segment::IndexedPoint:
				if (points)
					pointObjects(rect, rgnHdl, subdiv, seg.at(i), lbl, lblHdl,
					  points);
				break;
			case Segment::Line:
				if (lines)
					polyObjects(rect, rgnHdl, subdiv, seg.at(i), lbl, lblHdl,
					  net, netHdl, lines);
				break;
			case Segment::Polygon:
				if (polygons)
					polyObjects(rect, rgnHdl, subdiv, seg.at(i), lbl, lblHdl,
					  net, netHdl, polygons);
				break;
		}
	}
}

void RGNFile::extObjects(const RectC &rect, const SubDiv *subdiv, LBLFile *lbl,
  QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
  QList<IMG::Point> *points)
{
	Handle rgnHdl, lblHdl;

	if (!_init && !init())
		return;

	if (polygons && subdiv->polygonsOffset() != subdiv->polygonsEnd()) {
		quint32 start = _polygonsOffset + subdiv->polygonsOffset();
		quint32 end = subdiv->polygonsEnd()
		  ? _polygonsOffset + subdiv->polygonsEnd()
		  : _polygonsOffset + _polygonsSize;
		extPolyObjects(rect, rgnHdl, subdiv, Segment(start, end,
		  Segment::Polygon), lbl, lblHdl, polygons);
	}
	if (lines && subdiv->linesOffset() != subdiv->linesEnd()) {
		quint32 start = _linesOffset + subdiv->linesOffset();
		quint32 end = subdiv->linesEnd()
		  ? _linesOffset + subdiv->linesEnd()
		  : _linesOffset + _linesSize;
		extPolyObjects(rect, rgnHdl, subdiv, Segment(start, end, Segment::Line),
		  lbl, lblHdl, lines);
	}
	if (points && subdiv->pointsOffset() != subdiv->pointsEnd()) {
		quint32 start = _pointsOffset + subdiv->pointsOffset();
		quint32 end = subdiv->pointsEnd()
		  ? _pointsOffset + subdiv->pointsEnd()
		  : _pointsOffset + _pointsSize;
		extPointObjects(rect, rgnHdl, subdiv, Segment(start, end,
		  Segment::Point), lbl, lblHdl, points);
	}
}

QVector<RGNFile::Segment> RGNFile::segments(Handle &hdl, const SubDiv *subdiv)
  const
{
	if (subdiv->offset() == subdiv->end() || !(subdiv->objects() & 0xF0))
		return QVector<Segment>();

	quint32 offset = _offset + subdiv->offset();

	int no = 0;
	for (quint8 mask = 0x10; mask; mask <<= 1)
		if (subdiv->objects() & mask)
			no++;

	if (!seek(hdl, offset))
		return QVector<Segment>();

	QVector<Segment> ret;
	quint32 start = offset + 2 * (no - 1);
	quint16 po;
	int cnt = 0;

	for (quint8 mask = 0x10; mask; mask <<= 1) {
		if (subdiv->objects() & mask) {
			if (cnt) {
				if (!readUInt16(hdl, po))
					return QVector<Segment>();
				start = offset + po;
			}
			if (!ret.isEmpty())
				ret.last().setEnd(start);
			ret.append(Segment(start, (Segment::Type)mask));
			cnt++;
		}
	}

	ret.last().setEnd(subdiv->end() ? _offset + subdiv->end() : _offset + _size);

	return ret;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RGNFile::Segment &segment)
{
	dbg.nospace() << "Segment(" << segment.start() << ", " << segment.end()
	  << ", " << segment.type() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
