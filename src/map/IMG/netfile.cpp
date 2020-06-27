#include "bitstream.h"
#include "huffmanstream.h"
#include "subdiv.h"
#include "nodfile.h"
#include "netfile.h"


bool adjCnts(BitStream4R &bs, QVector<quint16> &cnts, quint16 &mask)
{
	quint32 val, cnt, bits;
	if (!bs.read(4, val))
		return false;

	cnt = ((val >> 2) & 3) + 2;
	bits = ((val * 2) & 6) + 4;
	mask = 1<<(3 + ((val * 2) & 6));
	if (cnt == 5) {
		if (!bs.read(8, cnt))
			return false;
		Q_ASSERT(cnt > 4);
	}

	if (cnt < 2)
		return false;
	cnts.resize(cnt - 1);
	for (int i = 0; i < cnts.size(); i++)
		if (!bs.read(bits, cnts[i]))
			return false;

	return true;
}

bool skipNodes(BitStream4R &bs, const QVector<quint16> &cnts, quint16 mask)
{
	for (int i = 0; i < cnts.size(); i++) {
		if (cnts.at(i) & mask) {
			quint32 v1, v2, v2b;
			if (!bs.readVuint32SM(v1, v2, v2b))
				return false;
			if (!bs.skip(v1))
				return false;
		}
	}

	return true;
}

bool seekToLevel(BitStream4R &bs, quint8 level)
{
	quint32 v1, v2, v2b;

	for (quint8 i = 1; i < level; ) {
		if (!bs.readVuint32SM(v1, v2, v2b))
			return false;
		if (!bs.skip(v1))
			return false;

		Q_ASSERT(!(v2 & 2));
		if (v2 & 2)
			return false;
		if (v2 & 1)
			i++;
	};

	return true;
}

bool seekToLine(BitStream4R &bs, quint8 line)
{
	quint32 v1, v2, v2b;

	for (quint8 i = 0; i < line; i++) {
		if (!bs.readVuint32SM(v1, v2, v2b))
			return false;
		if (!bs.skip(v1))
			return false;

		Q_ASSERT(!(v2 & 2));
		if (v2 & 2)
			return false;
	}

	return true;
}


bool NETFile::init(Handle &hdl)
{
	quint8 multiplier;
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size) && readUInt8(hdl, multiplier)))
		return false;

	if (hdrLen >= 0x47) {
		quint32 info;
		if (!(seek(hdl, _gmpOffset + 0x37) && readUInt32(hdl, info)))
			return false;
		_tableId = ((info >> 2) & 0xF);

		if (!(seek(hdl, _gmpOffset + 0x43) && readUInt32(hdl, _linksOffset)
		  && readUInt32(hdl, _linksSize) && readUInt8(hdl, _linksShift)))
			return false;
	}

	_multiplier = 1<<multiplier;

	return true;
}

bool NETFile::link(const SubDiv *subdiv, Handle &hdl, NODFile *nod,
  Handle &nodHdl, const NODFile::BlockInfo blockInfo, quint8 linkId,
  quint8 lineId, const HuffmanTable &table, QList<IMG::Poly> *lines)
{
	if (!_multiplier && !init(hdl))
		return false;

	// TODO
	if (!subdiv->level())
		return false;

	NODFile::LinkInfo linkInfo;
	if (!nod->linkInfo(nodHdl, blockInfo, linkId, linkInfo))
		return false;

	quint32 linkOffset = _linksOffset + (linkInfo.linkOffset << _linksShift);
	Q_ASSERT(linkOffset <= _linksOffset + _linksSize);

	quint8 s68 = (linkInfo.flags >> 0x12) & 1;
	quint8 s69 = (linkInfo.flags >> 0x11) & 1;
	quint8 s6a = (linkInfo.flags >> 0x13) & 1;

	if (s6a == 1) {
		QVector<quint16> ca;
		quint16 mask;

		if (!seek(hdl, linkOffset))
			return false;

		BitStream4R bs(*this, hdl, linkOffset - _linksOffset);
		quint32 size;

		if (!bs.readVUInt32(size))
			return false;

		if (s69 == 0) {
			if (!adjCnts(bs, ca, mask))
				return false;
		}
		if (s68 == 1) {
			quint32 v1, v2, v2b;
			if (!bs.readVuint32SM(v1, v2, v2b))
				return false;
			Q_ASSERT(v1);
			if (!bs.skip(v1))
				return false;
		}
		if (!skipNodes(bs, ca, mask))
			return false;
		if (!seekToLevel(bs, subdiv->level()))
			return false;
		if (!seekToLine(bs, lineId))
			return false;

		quint32 v1, v2, v2b;
		if (!bs.readVuint32SM(v1, v2, v2b))
			return false;
		bs.resize(v1);

		quint32 lon, lat;
		if (!(bs.read(0x12 - v2b, lon) && bs.read(16, lat)))
			return false;
		if (2 < v2b)
			lon |= (v2 >> 2) << (0x12U - v2b);

		QPoint pos = QPoint(LS(subdiv->lon(), 8) + LS((qint16)lon,
		  32-subdiv->bits()), LS(subdiv->lat(), 8) + LS((qint16)lat,
		  32-subdiv->bits()));
		Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));

		IMG::Poly poly;
		if (!nod->linkType(nodHdl, blockInfo, linkId, poly.type))
			return false;
		poly.boundingRect = RectC(c, c);
		poly.points.append(QPointF(c.lon(), c.lat()));

		Q_ASSERT(_tableId == table.id());
		HuffmanStreamR stream(bs, table);
		if (!stream.init())
			return false;
		qint32 lonDelta, latDelta;

		while (stream.readNext(lonDelta, latDelta)) {
			pos.rx() += LS(lonDelta, 32-subdiv->bits());
			if (pos.rx() < 0 && subdiv->lon() >= 0)
				pos.rx() = 0x7fffffff;
			pos.ry() += LS(latDelta, 32-subdiv->bits());

			Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			poly.boundingRect = poly.boundingRect.united(c);
		}

		lines->append(poly);
	}

	return true;
}

bool NETFile::lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset)
{
	if (!_multiplier && !init(hdl))
		return false;

	if (!(seek(hdl, _offset + netOffset * _multiplier)
	  && readUInt24(hdl, lblOffset)))
		return false;

	lblOffset &= 0x3FFFFF;

	return true;
}
