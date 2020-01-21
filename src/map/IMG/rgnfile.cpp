#include <cstring>
#include "common/rectc.h"
#include "common/garmin.h"
#include "deltastream.h"
#include "huffmanstream.h"
#include "lblfile.h"
#include "netfile.h"
#include "rgnfile.h"


bool RGNFile::skipClassFields(Handle &hdl) const
{
	quint8 flags;
	quint32 rs;

	if (!readUInt8(hdl, flags))
		return false;

	switch (flags >> 5) {
		case 4:
			rs = 1;
			break;
		case 5:
			rs = 2;
			break;
		case 6:
			rs = 3;
			break;
		case 7:
			if (!readVUInt32(hdl, rs))
				return false;
			break;
		default:
			rs = 0;
			break;
	}

	return seek(hdl, hdl.pos + rs);
}

bool RGNFile::skipLclFields(Handle &hdl, const quint32 flags[3],
  Segment::Type type) const
{
	quint32 bitfield = 0xFFFFFFFF;

	if (flags[0] & 0x20000000)
		if (!readVBitfield32(hdl, bitfield))
			return false;

	for (int i = 0; i < 29; i++) {
		if ((flags[0] >> i) & 1) {
			if (bitfield & 1) {
				quint32 m = flags[(i >> 4) + 1] >> ((i * 2) & 0x1e) & 3;
				switch (i) {
					case 5:
						if (m == 1 && type == Segment::Point) {
							quint16 u16;
							if (!readUInt16(hdl, u16))
								return false;
						}
						break;
					default:
						break;
				}
			}
			bitfield >>= 1;
		}
	}

	return true;
}

void RGNFile::clearFlags()
{
	memset(_polygonsFlags, 0, sizeof(_polygonsFlags));
	memset(_linesFlags, 0, sizeof(_linesFlags));
	memset(_pointsFlags, 0, sizeof(_pointsFlags));
}

bool RGNFile::init(Handle &hdl)
{
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size)))
		return false;

	if (hdrLen >= 0x68) {
		if (!(readUInt32(hdl, _polygonsOffset) && readUInt32(hdl, _polygonsSize)
		  && seek(hdl, _gmpOffset + 0x2D) && readUInt32(hdl, _polygonsFlags[0])
		  && readUInt32(hdl, _polygonsFlags[1]) && readUInt32(hdl, _polygonsFlags[2])
		  && readUInt32(hdl, _linesOffset) && readUInt32(hdl, _linesSize)
		  && seek(hdl, _gmpOffset + 0x49) && readUInt32(hdl, _linesFlags[0])
		  && readUInt32(hdl, _linesFlags[1]) && readUInt32(hdl, _linesFlags[2])
		  && readUInt32(hdl, _pointsOffset) && readUInt32(hdl, _pointsSize)
		  && seek(hdl, _gmpOffset + 0x65) && readUInt32(hdl, _pointsFlags[0])
		  && readUInt32(hdl, _pointsFlags[1]) && readUInt32(hdl, _pointsFlags[2])))
			return false;
	}

	if (hdrLen >= 0x7D) {
		quint32 dictOffset, dictSize, info;
		if (!(seek(hdl, _gmpOffset + 0x71) && readUInt32(hdl, dictOffset)
		  && readUInt32(hdl, dictSize) && readUInt32(hdl, info)))
			return false;

		if (dictSize && dictOffset && (info & 0x1E))
			if (!_huffmanTable.load(*this, hdl, dictOffset, dictSize,
			  ((info >> 1) & 0xF) - 1))
				return false;
	}

	_init = true;

	return true;
}

bool RGNFile::polyObjects(const RectC &rect, Handle &hdl, const SubDiv *subdiv,
  const Segment &segment, LBLFile *lbl, Handle &lblHdl, NETFile *net,
  Handle &netHdl, QList<IMG::Poly> *polys) const
{
	if (!seek(hdl, segment.start()))
		return false;

	quint32 labelPtr;
	quint8 type, len8, bitstreamInfo;
	qint16 lon, lat;
	quint16 len;

	while (hdl.pos < (int)segment.end()) {
		IMG::Poly poly;

		if (!(readUInt8(hdl, type) && readUInt24(hdl, labelPtr)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;
		if (type & 0x80) {
			if (!readUInt16(hdl, len))
				return false;
		} else {
			if (!readUInt8(hdl, len8))
				return false;
			len = len8;
		}
		if (!readUInt8(hdl, bitstreamInfo))
			return false;

		poly.type = (segment.type() == Segment::Polygon)
		  ? ((quint32)(type & 0x7F)) << 8 : ((quint32)(type & 0x3F)) << 8;


		QPoint pos(subdiv->lon() + ((qint32)lon<<(24-subdiv->bits())),
		  subdiv->lat() + ((qint32)lat<<(24-subdiv->bits())));
		Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
		RectC br(c, c);
		poly.points.append(QPointF(c.lon(), c.lat()));

		qint32 lonDelta, latDelta;
		DeltaStream stream(*this, hdl, len, bitstreamInfo, labelPtr & 0x400000,
		  false);
		while (stream.readNext(lonDelta, latDelta)) {
			pos.rx() += lonDelta<<(24-subdiv->bits());
			pos.ry() += latDelta<<(24-subdiv->bits());

			Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			br = br.united(c);
		}
		if (!(stream.atEnd() && stream.flush()))
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
  const SubDiv *subdiv, quint32 shift, const Segment &segment, LBLFile *lbl,
  Handle &lblHdl, QList<IMG::Poly> *polys, bool line) const
{
	quint32 labelPtr, len;
	quint8 type, subtype;
	qint16 lon, lat;


	if (!seek(hdl, segment.start()))
		return false;

	while (hdl.pos < (int)segment.end()) {
		IMG::Poly poly;
		QPoint pos;
		RectC br;

		if (!(readUInt8(hdl, type) && readUInt8(hdl, subtype)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)
		  && readVUInt32(hdl, len)))
			return false;

		poly.type = 0x10000 | (quint16(type)<<8) | (subtype & 0x1F);
		labelPtr = 0;

		if (!_huffmanTable.isNull()) {
			pos = QPoint((subdiv->lon()<<8) + ((qint32)lon<<(32-subdiv->bits())),
			  (subdiv->lat()<<8) + ((qint32)lat<<(32-subdiv->bits())));

			qint32 lonDelta, latDelta;
			HuffmanStream stream(*this, hdl, len, _huffmanTable, line);

			if (shift) {
				if (!stream.readOffset(lonDelta, latDelta))
					return false;
				pos = QPoint(pos.x() | lonDelta<<(32-subdiv->bits()-shift),
				  pos.y() | latDelta<<(32-subdiv->bits()-shift));
			}
			Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
			br = RectC(c, c);
			poly.points.append(QPointF(c.lon(), c.lat()));

			while (stream.readNext(lonDelta, latDelta)) {
				pos.rx() += lonDelta<<(32-subdiv->bits()-shift);
				pos.ry() += latDelta<<(32-subdiv->bits()-shift);

				Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
				poly.points.append(QPointF(c.lon(), c.lat()));
				br = br.united(c);
			}

			if (!(stream.atEnd() && stream.flush()))
				return false;
		} else {
			pos = QPoint(subdiv->lon() + ((qint32)lon<<(24-subdiv->bits())),
			  subdiv->lat() + ((qint32)lat<<(24-subdiv->bits())));
			Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
			br = RectC(c, c);
			poly.points.append(QPointF(c.lon(), c.lat()));

			quint8 bitstreamInfo;
			if (!readUInt8(hdl, bitstreamInfo))
				return false;

			qint32 lonDelta, latDelta;
			DeltaStream stream(*this, hdl, len - 1, bitstreamInfo, false, true);

			while (stream.readNext(lonDelta, latDelta)) {
				pos.rx() += lonDelta<<(24-subdiv->bits());
				pos.ry() += latDelta<<(24-subdiv->bits());

				Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
				poly.points.append(QPointF(c.lon(), c.lat()));
				br = br.united(c);
			}
			if (!(stream.atEnd() && stream.flush()))
				return false;
		}

		if (subtype & 0x20 && !readUInt24(hdl, labelPtr))
			return false;
		if (subtype & 0x80 && !skipClassFields(hdl))
			return false;
		if (subtype & 0x40 && !skipLclFields(hdl, line ? _linesFlags
		  : _polygonsFlags, segment.type()))
			return false;

		if (!rect.intersects(br))
			continue;

		if (lbl && (labelPtr & 0x3FFFFF))
			poly.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF);

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

		if (!(readUInt8(hdl, type) && readUInt24(hdl, labelPtr)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;
		if (labelPtr & 0x800000) {
			if (!readUInt8(hdl, subtype))
				return false;
		} else
			subtype = 0;

		point.type = (quint16)type<<8 | subtype;

		qint16 lonOffset = lon<<(24-subdiv->bits());
		qint16 latOffset = lat<<(24-subdiv->bits());
		point.coordinates = Coordinates(toWGS24(subdiv->lon() + lonOffset),
		  toWGS24(subdiv->lat() + latOffset));

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
  const SubDiv *subdiv, const Segment &segment, LBLFile *lbl,
  Handle &lblHdl, QList<IMG::Point> *points) const
{
	quint8 type, subtype;
	qint16 lon, lat;
	quint32 labelPtr;

	if (!seek(hdl, segment.start()))
		return false;

	while (hdl.pos < (int)segment.end()) {
		IMG::Point point;

		if (!(readUInt8(hdl, type) && readUInt8(hdl, subtype)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;

		point.type = 0x10000 | (((quint32)type)<<8) | (subtype & 0x1F);

		qint16 lonOffset = lon<<(24-subdiv->bits());
		qint16 latOffset = lat<<(24-subdiv->bits());
		point.coordinates = Coordinates(toWGS24(subdiv->lon() + lonOffset),
		  toWGS24(subdiv->lat() + latOffset));
		labelPtr = 0;

		if (subtype & 0x20 && !readUInt24(hdl, labelPtr))
			return false;
		if (subtype & 0x80 && !skipClassFields(hdl))
			return false;
		if (subtype & 0x40 && !skipLclFields(hdl, _pointsFlags, segment.type()))
			return false;

		if (!rect.contains(point.coordinates))
			continue;

		point.poi = labelPtr & 0x400000;
		if (lbl && (labelPtr & 0x3FFFFF)) {
			point.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF, point.poi);
			point.id = ((quint64)point.type)<<40
			  | ((quint64)lbl->offset())<<24 | (labelPtr & 0x3FFFFF);
		}

		points->append(point);
	}

	return true;
}

void RGNFile::objects(const RectC &rect, const SubDiv *subdiv,
  LBLFile *lbl, NETFile *net, QList<IMG::Poly> *polygons,
  QList<IMG::Poly> *lines, QList<IMG::Point> *points)
{
	Handle rgnHdl, lblHdl, netHdl;

	if (!_init && !init(rgnHdl))
		return;

	QVector<Segment> seg(segments(rgnHdl, subdiv));

	for (int i = 0; i < seg.size(); i++) {
		const Segment &segment = seg.at(i);

		if (segment.start() == segment.end())
			continue;

		switch (segment.type()) {
			case Segment::Point:
			case Segment::IndexedPoint:
				if (points)
					pointObjects(rect, rgnHdl, subdiv, segment, lbl, lblHdl,
					  points);
				break;
			case Segment::Line:
				if (lines)
					polyObjects(rect, rgnHdl, subdiv, segment, lbl, lblHdl, net,
					  netHdl, lines);
				break;
			case Segment::Polygon:
				if (polygons)
					polyObjects(rect, rgnHdl, subdiv, segment, lbl, lblHdl, net,
					  netHdl, polygons);
				break;
			case Segment::RoadReference:
				break;
		}
	}
}

void RGNFile::extObjects(const RectC &rect, const SubDiv *subdiv, quint32 shift,
  LBLFile *lbl, QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
  QList<IMG::Point> *points)
{
	Handle rgnHdl, lblHdl;

	if (!_init && !init(rgnHdl))
		return;
	if (polygons && subdiv->polygonsOffset() != subdiv->polygonsEnd()) {
		quint32 start = _polygonsOffset + subdiv->polygonsOffset();
		quint32 end = subdiv->polygonsEnd()
		  ? _polygonsOffset + subdiv->polygonsEnd()
		  : _polygonsOffset + _polygonsSize;
		extPolyObjects(rect, rgnHdl, subdiv, shift, Segment(start, end,
		  Segment::Polygon), lbl, lblHdl, polygons, false);
	}
	if (lines && subdiv->linesOffset() != subdiv->linesEnd()) {
		quint32 start = _linesOffset + subdiv->linesOffset();
		quint32 end = subdiv->linesEnd()
		  ? _linesOffset + subdiv->linesEnd()
		  : _linesOffset + _linesSize;
		extPolyObjects(rect, rgnHdl, subdiv, shift, Segment(start, end,
		  Segment::Line), lbl, lblHdl, lines, true);
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
	if (subdiv->offset() == subdiv->end() || !(subdiv->objects() & 0x1F))
		return QVector<Segment>();

	quint32 offset = _offset + subdiv->offset();

	int no = 0;
	for (quint8 mask = 0x1; mask <= 0x10; mask <<= 1)
		if (subdiv->objects() & mask)
			no++;

	if (!seek(hdl, offset))
		return QVector<Segment>();

	QVector<Segment> ret;
	quint32 start = offset + 2 * (no - 1);
	quint16 po;
	int cnt = 0;

	for (quint16 mask = 0x1; mask <= 0x10; mask <<= 1) {
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
	QString type;
	switch (segment.type()) {
		case RGNFile::Segment::Point:
			type = "Point";
			break;
		case RGNFile::Segment::IndexedPoint:
			type = "IndexedPoint";
			break;
		case RGNFile::Segment::Line:
			type = "Line";
			break;
		case RGNFile::Segment::Polygon:
			type = "Polygon";
			break;
		case RGNFile::Segment::RoadReference:
			type = "RoadReference";
			break;
	}

	dbg.nospace() << "Segment(" << segment.start() << ", " << segment.end()
	  - segment.start() << ", " << type << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
