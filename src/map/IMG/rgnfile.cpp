#include "common/rectc.h"
#include "common/garmin.h"
#include "deltastream.h"
#include "huffmanstream.h"
#include "lblfile.h"
#include "netfile.h"
#include "nodfile.h"
#include "rgnfile.h"


#define MASK(bits) ((2U << ((bits) - 1U)) - 1U)

static quint64 pointId(const QPoint &pos, quint32 type, quint32 labelPtr)
{
	quint64 id;

	uint hash = qHash(QPair<uint,uint>(qHash(QPair<int, int>(pos.x(),
	  pos.y())), labelPtr & 0x3FFFFF));
	id = ((quint64)type)<<32 | hash;
	// Make country labels precedent over city labels
	if (!(type >= 0x1400 && type <= 0x153f))
		id |= 1ULL<<63;

	return id;
}

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

	return seek(hdl, pos(hdl) + rs);
}

bool RGNFile::skipLclFields(Handle &hdl, const quint32 flags[3]) const
{
	quint32 bitfield = 0xFFFFFFFF;

	if (flags[0] & 0x20000000)
		if (!readVBitfield32(hdl, bitfield))
			return false;

	for (int i = 0, j = 0; i < 29; i++) {
		if ((flags[0] >> i) & 1) {
			if (bitfield & 1) {
				quint32 m = flags[(j >> 4) + 1] >> ((j * 2) & 0x1e) & 3;

				quint32 skip = 0;
				if (m == 3) {
					if (!readVUInt32(hdl, skip))
						return false;
				} else
					skip = m + 1;
				if (!seek(hdl, pos(hdl) + skip))
					return false;
			}
			bitfield >>= 1;
			j++;
		}
	}

	return true;
}

bool RGNFile::skipGblFields(Handle &hdl, quint32 flags) const
{
	int cnt = 0;

	do {
		cnt = cnt + (flags & 3);
		flags = flags >> 2;
	} while (flags != 0);

	return seek(hdl, pos(hdl) + cnt);
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
		  && seek(hdl, _gmpOffset + 0x29) && readUInt32(hdl, _polygonsGblFlags)
		  && readUInt32(hdl, _polygonsLclFlags[0])
		  && readUInt32(hdl, _polygonsLclFlags[1])
		  && readUInt32(hdl, _polygonsLclFlags[2])
		  && readUInt32(hdl, _linesOffset) && readUInt32(hdl, _linesSize)
		  && seek(hdl, _gmpOffset + 0x45) && readUInt32(hdl, _linesGblFlags)
		  && readUInt32(hdl, _linesLclFlags[0])
		  && readUInt32(hdl, _linesLclFlags[1])
		  && readUInt32(hdl, _linesLclFlags[2])
		  && readUInt32(hdl, _pointsOffset) && readUInt32(hdl, _pointsSize)
		  && seek(hdl, _gmpOffset + 0x61) && readUInt32(hdl, _pointsGblFlags)
		  && readUInt32(hdl, _pointsLclFlags[0])
		  && readUInt32(hdl, _pointsLclFlags[1])
		  && readUInt32(hdl, _pointsLclFlags[2])))
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

bool RGNFile::polyObjects(Handle &hdl, const SubDiv *subdiv,
  SegmentType segmentType, LBLFile *lbl, Handle &lblHdl, NETFile *net,
  Handle &netHdl, QList<IMG::Poly> *polys) const
{
	const SubDiv::Segment &segment = (segmentType == Line)
	 ? subdiv->lines() : subdiv->polygons();

	if (!segment.isValid())
		return true;
	if (!seek(hdl, segment.offset()))
		return false;

	quint32 labelPtr;
	quint8 type, len8, bitstreamInfo;
	qint16 lon, lat;
	quint16 len;

	while (pos(hdl) < segment.end()) {
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

		poly.type = (segmentType == Polygon)
		  ? ((quint32)(type & 0x7F)) << 8 : ((quint32)(type & 0x3F)) << 8;


		QPoint pos(subdiv->lon() + LS(lon, 24-subdiv->bits()),
		  subdiv->lat() + LS(lat, 24-subdiv->bits()));
		Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
		poly.boundingRect = RectC(c, c);
		poly.points.append(QPointF(c.lon(), c.lat()));

		qint32 lonDelta, latDelta;
		DeltaStream stream(*this, hdl, len, bitstreamInfo, labelPtr & 0x400000,
		  false);
		while (stream.readNext(lonDelta, latDelta)) {
			pos.rx() += LS(lonDelta, (24-subdiv->bits()));
			if (pos.rx() >= 0x800000 && subdiv->lon() >= 0)
				pos.rx() = 0x7fffff;
			pos.ry() += LS(latDelta, (24-subdiv->bits()));

			Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			poly.boundingRect = poly.boundingRect.united(c);
		}
		if (!(stream.atEnd() && stream.flush()))
			return false;

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

bool RGNFile::extPolyObjects(Handle &hdl, const SubDiv *subdiv, quint32 shift,
  SegmentType segmentType, LBLFile *lbl, Handle &lblHdl,
  QList<IMG::Poly> *polys) const
{
	quint32 labelPtr, len;
	quint8 type, subtype;
	qint16 lon, lat;
	const SubDiv::Segment &segment = (segmentType == Line)
	 ? subdiv->extLines() : subdiv->extPolygons();


	if (!segment.isValid())
		return true;
	if (!seek(hdl, segment.offset()))
		return false;

	while (pos(hdl) < segment.end()) {
		IMG::Poly poly;
		QPoint pos;

		if (!(readUInt8(hdl, type) && readUInt8(hdl, subtype)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)
		  && readVUInt32(hdl, len)))
			return false;
		Q_ASSERT(SubFile::pos(hdl) + len <= segment.end());

		poly.type = 0x10000 | (quint16(type)<<8) | (subtype & 0x1F);
		labelPtr = 0;

		if (!_huffmanTable.isNull()) {
			pos = QPoint(LS(subdiv->lon(), 8) + LS(lon, 32-subdiv->bits()),
			  LS(subdiv->lat(), 8) + LS(lat, (32-subdiv->bits())));

			qint32 lonDelta, latDelta;
			BitStream4F bs(*this, hdl, len);
			HuffmanStreamF stream(bs, _huffmanTable);
			if (!stream.init(segmentType == Line))
				return false;

			if (shift) {
				if (!stream.readOffset(lonDelta, latDelta))
					return false;
				pos = QPoint(pos.x() | LS(lonDelta, 32-subdiv->bits()-shift),
				  pos.y() | LS(latDelta, 32-subdiv->bits()-shift));
			}
			Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
			poly.boundingRect = RectC(c, c);
			poly.points.append(QPointF(c.lon(), c.lat()));

			while (stream.readNext(lonDelta, latDelta)) {
				pos.rx() += LS(lonDelta, 32-subdiv->bits()-shift);
				if (pos.rx() < 0 && subdiv->lon() >= 0)
					pos.rx() = 0x7fffffff;
				pos.ry() += LS(latDelta, 32-subdiv->bits()-shift);

				Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
				poly.points.append(QPointF(c.lon(), c.lat()));
				poly.boundingRect = poly.boundingRect.united(c);
			}

			if (!(stream.atEnd() && stream.flush()))
				return false;
		} else {
			pos = QPoint(subdiv->lon() + LS(lon, 24-subdiv->bits()),
			  subdiv->lat() + LS(lat, 24-subdiv->bits()));
			Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
			poly.boundingRect = RectC(c, c);
			poly.points.append(QPointF(c.lon(), c.lat()));

			quint8 bitstreamInfo;
			if (!readUInt8(hdl, bitstreamInfo))
				return false;

			qint32 lonDelta, latDelta;
			DeltaStream stream(*this, hdl, len - 1, bitstreamInfo, false, true);

			while (stream.readNext(lonDelta, latDelta)) {
				pos.rx() += LS(lonDelta, 24-subdiv->bits());
				if (pos.rx() >= 0x800000 && subdiv->lon() >= 0)
					pos.rx() = 0x7fffff;
				pos.ry() += LS(latDelta, 24-subdiv->bits());

				Coordinates c(toWGS24(pos.x()), toWGS24(pos.y()));
				poly.points.append(QPointF(c.lon(), c.lat()));
				poly.boundingRect = poly.boundingRect.united(c);
			}
			if (!(stream.atEnd() && stream.flush()))
				return false;
		}

		if (subtype & 0x20 && !readUInt24(hdl, labelPtr))
			return false;
		if (subtype & 0x80 && !skipClassFields(hdl))
			return false;
		if (subtype & 0x40 && !skipLclFields(hdl, segmentType == Line
		  ? _linesLclFlags : _polygonsLclFlags))
			return false;
		quint32 gblFlags = (segmentType == Line)
		  ? _linesGblFlags : _polygonsGblFlags;
		if (gblFlags && !skipGblFields(hdl, gblFlags))
			return false;

		if (lbl && (labelPtr & 0x3FFFFF))
			poly.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF);

		polys->append(poly);
	}

	return true;
}

bool RGNFile::pointObjects(Handle &hdl, const SubDiv *subdiv,
  SegmentType segmentType, LBLFile *lbl, Handle &lblHdl,
  QList<IMG::Point> *points) const
{
	const SubDiv::Segment &segment = (segmentType == IndexedPoint)
	 ? subdiv->idxPoints() : subdiv->points();


	if (!segment.isValid())
		return true;
	if (!seek(hdl, segment.offset()))
		return false;

	while (pos(hdl) < segment.end()) {
		IMG::Point point;
		quint8 type, subtype;
		qint16 lon, lat;
		quint32 labelPtr;

		if (!(readUInt8(hdl, type) && readUInt24(hdl, labelPtr)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;
		if (labelPtr & 0x800000) {
			if (!readUInt8(hdl, subtype))
				return false;
		} else
			subtype = 0;

		QPoint pos(subdiv->lon() + LS(lon, 24-subdiv->bits()),
		  subdiv->lat() + LS(lat, 24-subdiv->bits()));

		point.type = (quint16)type<<8 | subtype;
		point.coordinates = Coordinates(toWGS24(pos.x()), toWGS24(pos.y()));
		point.id = pointId(pos, point.type, labelPtr & 0x3FFFFF);
		point.poi = labelPtr & 0x400000;
		if (lbl && (labelPtr & 0x3FFFFF))
			point.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF, point.poi,
			  !(point.type == 0x1400 || point.type == 0x1500
			  || point.type == 0x1e00));

		points->append(point);
	}

	return true;
}

bool RGNFile::extPointObjects(Handle &hdl, const SubDiv *subdiv, LBLFile *lbl,
  Handle &lblHdl, QList<IMG::Point> *points) const
{
	const SubDiv::Segment &segment = subdiv->extPoints();

	if (!segment.isValid())
		return true;
	if (!seek(hdl, segment.offset()))
		return false;

	while (pos(hdl) < segment.end()) {
		IMG::Point point;
		qint16 lon, lat;
		quint8 type, subtype;
		quint32 labelPtr = 0;

		if (!(readUInt8(hdl, type) && readUInt8(hdl, subtype)
		  && readInt16(hdl, lon) && readInt16(hdl, lat)))
			return false;

		if (subtype & 0x20 && !readUInt24(hdl, labelPtr))
			return false;
		if (subtype & 0x80 && !skipClassFields(hdl))
			return false;
		if (subtype & 0x40 && !skipLclFields(hdl, _pointsLclFlags))
			return false;
		if (_pointsGblFlags && !skipGblFields(hdl, _pointsGblFlags))
			return false;

		QPoint pos(subdiv->lon() + LS(lon, 24-subdiv->bits()),
		  subdiv->lat() + LS(lat, 24-subdiv->bits()));

		point.type = 0x10000 | (((quint32)type)<<8) | (subtype & 0x1F);
		// Discard NT points breaking style draw order logic (and causing huge
		// performance drawback)
		if (point.type == 0x11400)
			continue;

		point.coordinates = Coordinates(toWGS24(pos.x()), toWGS24(pos.y()));
		point.id = pointId(pos, point.type, labelPtr & 0x3FFFFF);
		point.poi = labelPtr & 0x400000;
		if (lbl && (labelPtr & 0x3FFFFF))
			point.label = lbl->label(lblHdl, labelPtr & 0x3FFFFF, point.poi);

		points->append(point);
	}

	return true;
}

bool RGNFile::links(Handle &hdl, const SubDiv *subdiv, NETFile *net,
  Handle &netHdl, NODFile *nod, Handle &nodHdl, QList<IMG::Poly> *lines) const
{
	quint32 size, blockIndexIdSize, blockIndexId;
	quint8 flags;
	const SubDiv::Segment &segment = subdiv->roadReferences();

	if (!segment.isValid())
		return true;
	if (!seek(hdl, segment.offset()))
		return false;

	if (!net || !nod)
		return false;
	if (!(blockIndexIdSize = nod->indexIdSize(nodHdl)))
		return false;

	while (pos(hdl) < segment.end()) {
		if (!readVUInt32(hdl, size))
			return false;

		quint32 entryStart = pos(hdl);

		if (!(readUInt8(hdl, flags) && readVUInt32(hdl, blockIndexIdSize,
		  blockIndexId)))
			return false;

		quint8 bits[3];
		for (int i = 0; i < 3; i++)
			bits[i] = 0x4000a08 >> (((flags >> (2*i) & 3) << 3) ^ 0x10);
		quint8 byteSize = ((bits[0] + bits[1] + bits[2]) + 7) >> 3;

		quint32 counts;
		if (!readVUInt32(hdl, byteSize, counts))
			return false;

		quint16 b8 = bits[0] ? (MASK(bits[0]) & counts) + 1 : 0;
		quint16 b10 = bits[1] ? (MASK(bits[1]) & (counts >> bits[0])) + 1 : 0;
		quint16 b16 = bits[2] ? (MASK(bits[2]) & (counts >> (bits[0] + bits[1])))
		  + 1 : 0;

		NODFile::BlockInfo blockInfo;
		if (!nod->blockInfo(nodHdl, blockIndexId, blockInfo))
			return false;

		quint8 linkId, lineId;
		for (int i = 0; i < b8 + b10 + b16; i++) {
			if (!b8 || b8 <= i) {
				quint16 v16;
				if (!readUInt16(hdl, v16))
					return false;

				if (!b16 || b8 + b16 <= i) {
					int shift = ((i - (b8 + b16)) * 10) % 8;
					linkId = (quint8)(v16 >> shift);
					lineId = (((v16 >> shift) >> 8) & 3) + 1;

					if (shift < 6 && i < b8 + b10 + b16 - 1)
						seek(hdl, pos(hdl) - 1);
				} else {
					linkId = (quint8)v16;
					lineId = v16 >> 8;
					Q_ASSERT(lineId > 4);
				}
			} else {
				if (!readUInt8(hdl, linkId))
					return false;
				lineId = 0;
			}

			net->link(subdiv, netHdl, nod, nodHdl, blockInfo, linkId, lineId,
			  _huffmanTable, lines);
		}

		Q_ASSERT(entryStart + size == pos(hdl));
	}

	return true;
}

QMap<RGNFile::SegmentType, SubDiv::Segment> RGNFile::segments(Handle &hdl,
  SubDiv *subdiv) const
{
	QMap<SegmentType, SubDiv::Segment> ret;

	if (subdiv->offset() == subdiv->end() || !(subdiv->objects() & 0x1F))
		return ret;

	quint32 offset = _offset + subdiv->offset();

	int no = 0;
	for (quint8 mask = 0x1; mask <= 0x10; mask <<= 1)
		if (subdiv->objects() & mask)
			no++;

	if (!seek(hdl, offset))
		return ret;

	quint32 start = offset + 2 * (no - 1);
	quint32 ls = 0;
	SegmentType lt = (SegmentType)0;

	for (quint8 mask = 0x1; mask <= 0x10; mask <<= 1) {
		if (subdiv->objects() & mask) {
			if (ls) {
				quint16 po;
				if (!readUInt16(hdl, po) || !po)
					return QMap<RGNFile::SegmentType, SubDiv::Segment>();
				start = offset + po;
				ret.insert(lt, SubDiv::Segment(ls, start));
			}

			lt = (SegmentType)mask;
			ls = start;
		}
	}

	ret.insert(lt, SubDiv::Segment(ls, subdiv->end()
	  ? _offset + subdiv->end() : _offset + _size));

	return ret;
}

bool RGNFile::subdivInit(Handle &hdl, SubDiv *subdiv) const
{
	QMap<RGNFile::SegmentType, SubDiv::Segment> seg(segments(hdl, subdiv));
	SubDiv::Segment extPoints, extLines, extPolygons;

	if (subdiv->extPointsOffset() != subdiv->extPointsEnd()) {
		quint32 start = _pointsOffset + subdiv->extPointsOffset();
		quint32 end = subdiv->extPointsEnd()
		  ? _pointsOffset + subdiv->extPointsEnd()
		  : _pointsOffset + _pointsSize;
		extPoints = SubDiv::Segment(start, end);
	}
	if (subdiv->extPolygonsOffset() != subdiv->extPolygonsEnd()) {
		quint32 start = _polygonsOffset + subdiv->extPolygonsOffset();
		quint32 end = subdiv->extPolygonsEnd()
		  ? _polygonsOffset + subdiv->extPolygonsEnd()
		  : _polygonsOffset + _polygonsSize;
		extPolygons = SubDiv::Segment(start, end);
	}
	if (subdiv->extLinesOffset() != subdiv->extLinesEnd()) {
		quint32 start = _linesOffset + subdiv->extLinesOffset();
		quint32 end = subdiv->extLinesEnd()
		  ? _linesOffset + subdiv->extLinesEnd()
		  : _linesOffset + _linesSize;
		extLines = SubDiv::Segment(start, end);
	}

	subdiv->init(seg.value(Point), seg.value(IndexedPoint), seg.value(Line),
	  seg.value(Polygon), seg.value(RoadReference), extPoints, extLines,
	  extPolygons);

	return true;
}
