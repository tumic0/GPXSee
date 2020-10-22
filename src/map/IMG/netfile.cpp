#include "bitstream.h"
#include "huffmanstream.h"
#include "subdiv.h"
#include "nodfile.h"
#include "lblfile.h"
#include "netfile.h"


static bool readAdjCounts(BitStream4R &bs, QVector<quint16> &cnts, quint16 &mask)
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
	}

	if (cnt < 2)
		return false;
	cnts.resize(cnt - 1);
	for (int i = 0; i < cnts.size(); i++)
		if (!bs.read(bits, cnts[i]))
			return false;

	return true;
}

static bool skipShape(BitStream4R &bs)
{
	quint32 v1, v2, v2b;

	if (!bs.readVuint32SM(v1, v2, v2b))
		return false;

	return bs.skip(v1);
}

static bool skipAdjShapes(BitStream4R &bs, const QVector<quint16> &cnts,
  quint16 mask, bool firstIsShape)
{
	if (firstIsShape && !skipShape(bs))
		return false;

	for (int i = 0; i < cnts.size(); i++) {
		if (cnts.at(i) & mask) {
			if (!skipShape(bs))
				return false;
		}
	}

	return true;
}

static bool seekToLevel(BitStream4R &bs, quint8 level)
{
	quint32 v1, v2, v2b;

	for (quint8 i = 1; i < level; ) {
		if (!bs.readVuint32SM(v1, v2, v2b))
			return false;
		if (!bs.skip(v1))
			return false;

		if (v2 & 2)
			return false;
		if (v2 & 1)
			i++;
	};

	return true;
}

static bool seekToLine(BitStream4R &bs, quint8 line)
{
	quint32 v1, v2, v2b;

	for (quint8 i = 0; i < line; i++) {
		if (!bs.readVuint32SM(v1, v2, v2b))
			return false;
		if (!bs.skip(v1))
			return false;

		if (v2 & 2)
			return false;
	}

	return true;
}

static bool readLine(BitStream4R &bs, const SubDiv *subdiv,
  const HuffmanTable &table, IMG::Poly &poly)
{
	quint32 v1, v2, v2b;
	if (!bs.readVuint32SM(v1, v2, v2b))
		return false;
	bs.resize(v1);

	quint32 lon, lat;
	if (!(bs.read(0x12 - v2b, lon) && bs.read(16, lat)))
		return false;
	if (2 < v2b)
		lon |= (v2 >> 2) << (0x12U - v2b);

	QPoint pos = QPoint(LS(subdiv->lon(), 8) + LS((qint16)lon, 32-subdiv->bits()),
	  LS(subdiv->lat(), 8) + LS((qint16)lat, 32-subdiv->bits()));
	Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));

	poly.boundingRect = RectC(c, c);
	poly.points.append(QPointF(c.lon(), c.lat()));

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

	return stream.atEnd();
}

static bool readNodeGeometry(NODFile *nod, SubFile::Handle &nodHdl,
  NODFile::AdjacencyInfo &adj, IMG::Poly &poly, quint16 cnt = 0xFFFF)
{
	for (int i = 0; i <= cnt; i++) {
		int ret = nod->nextNode(nodHdl, adj);
		if (ret < 0)
			return false;
		else if (ret > 0)
			return (cnt == 0xFFFF);

		Coordinates c(toWGS32(adj.nodeInfo.pos.x()),
		  toWGS32(adj.nodeInfo.pos.y()));
		poly.points.append(QPointF(c.lon(), c.lat()));
		poly.boundingRect = poly.boundingRect.united(c);
	}

	return true;
}

static bool skipNodes(NODFile *nod, SubFile::Handle &nodHdl,
  NODFile::AdjacencyInfo &adj, int cnt)
{
	for (int i = 0; i < cnt; i++)
		if (nod->nextNode(nodHdl, adj))
			return false;

	return true;
}

static bool readShape(NODFile *nod, SubFile::Handle &nodHdl,
  NODFile::AdjacencyInfo &adj, BitStream4R &bs, const HuffmanTable &table,
  const SubDiv *subdiv, quint32 shift, IMG::Poly &poly, quint16 cnt = 0xFFFF,
  bool check = false)
{
	quint32 v1, v2, v2b;
	if (!bs.readVuint32SM(v1, v2, v2b))
		return false;
	BitStream4R::State state;
	bs.save(state);
	bs.resize(v1);

	quint32 flags;
	if (!bs.read(8, flags))
		return false;
	flags |= (v2 << 8);

	bool hasCoordinatesAdjustBit = flags & (1 << (v2b + 7));
	bool useEosBit = flags & (1 << (v2b + 5));
	bool startWithStream = flags & (1 << (v2b + 6));

	quint32 extraBits;
	int lonSign, latSign;

	if ((flags >> (v2b + 4) & 1) == 0) {
		extraBits = v2b + 4;
		lonSign = 0;
	} else {
		extraBits = v2b + 3;
		lonSign = 1;
		if ((flags >> (v2b + 3) & 1) != 0) {
			lonSign = -1;
		}
	}
	extraBits -= 1;
	if ((flags >> extraBits & 1) == 0) {
		latSign = 0;
	} else {
		extraBits -= 1;
		latSign = -1;
		if ((flags >> extraBits & 1) == 0) {
			latSign = 1;
		}
	}

	if (nod->nextNode(nodHdl, adj))
		return false;
	QPoint pos(adj.nodeInfo.pos);
	quint16 nodes = 0;

	if (!startWithStream) {
		Coordinates c(toWGS32(adj.nodeInfo.pos.x()),
		  toWGS32(adj.nodeInfo.pos.y()));
		poly.points.append(QPointF(c.lon(), c.lat()));
		poly.boundingRect = poly.boundingRect.united(c);

		while (!adj.eog) {
			int ret = nod->nextNode(nodHdl, adj);
			if (ret < 0)
				return false;
			else if (ret > 0)
				break;
			nodes++;

			c = Coordinates(toWGS32(adj.nodeInfo.pos.x()),
			  toWGS32(adj.nodeInfo.pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			poly.boundingRect = poly.boundingRect.united(c);
			pos = adj.nodeInfo.pos;
		}
	}

	HuffmanStreamR stream(bs, table);
	if (!stream.init(lonSign, latSign, flags, extraBits))
		return false;
	qint32 lonDelta, latDelta;
	QVector<QPoint> deltas;

	quint32 adjustBit = 0;
	quint32 stepsCnt = 0;
	quint32 steps = 0;
	quint32 eos = 0;

	while (true) {
		if ((stepsCnt == steps) && !useEosBit) {
			if (!stream.readSymbol(steps))
				break;
			if (!steps)
				break;
		}

		if (!stream.readNext(lonDelta, latDelta))
			break;

		if (hasCoordinatesAdjustBit && !stream.read(1, adjustBit))
			return false;

		stepsCnt++;

		if (useEosBit) {
			if (!stream.read(1, eos))
				return false;
		} else {
			if (steps == stepsCnt)
				eos = 1;
		}

		if (!startWithStream) {
			pos.rx() += LS(lonDelta, 32-subdiv->bits()-shift);
			pos.ry() += LS(latDelta, 32-subdiv->bits()-shift);

			Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			poly.boundingRect = poly.boundingRect.united(c);
		} else {
			deltas.append(QPoint(lonDelta, latDelta));
			poly.points.append(QPointF());
		}

		if (startWithStream && eos) {
			for (int i = deltas.size() - 1, j = 0; i >= 0; i--, j++) {
				pos.rx() -= LS(deltas.at(i).x(), 32-subdiv->bits()-shift);
				pos.ry() -= LS(deltas.at(i).y(), 32-subdiv->bits()-shift);

				Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
				poly.points[poly.points.size() - 1 - j] = QPointF(c.lon(), c.lat());
				poly.boundingRect = poly.boundingRect.united(c);
			}

			pos = adj.nodeInfo.pos;
			Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
			poly.points.append(QPointF(c.lon(), c.lat()));
			poly.boundingRect = poly.boundingRect.united(c);

			stepsCnt = 0;
			steps = 0;
			startWithStream = false;

			if (adj.eog)
				eos = 0;
		}

		if (eos) {
			if (nodes >= cnt)
				break;

			do {
				int ret = nod->nextNode(nodHdl, adj);
				if (ret < 0)
					return false;
				else if (ret > 0)
					break;
				nodes++;

				if (check && nodes == cnt) {
					if (!(bs.restore(state) && bs.skip(v1)
					  && bs.readVuint32SM(v1, v2, v2b)))
						return false;
					if (5 < v2b)
						v2 >>= v2b - 2;
					if (v2 & 1)
						break;
				}

				Coordinates c(toWGS32(adj.nodeInfo.pos.x()),
				  toWGS32(adj.nodeInfo.pos.y()));
				poly.points.append(QPointF(c.lon(), c.lat()));
				poly.boundingRect = poly.boundingRect.united(c);
				pos = adj.nodeInfo.pos;
			} while (!adj.eog && nodes < cnt);

			if (nodes == cnt)
				break;

			steps = 0;
			stepsCnt = 0;
			eos = 0;
		}
	}

	return true;
}


bool NETFile::linkLabel(Handle &hdl, quint32 offset, quint32 size, LBLFile *lbl,
  Handle &lblHdl, Label &label)
{
	if (!seek(hdl, offset))
		return false;
	BitStream1 bs(*this, hdl, size);

	quint32 flags, labelPtr;
	if (!bs.read(8, flags))
		return false;
	if (!(flags & 1))
		return true;

	if (!bs.readUInt24(labelPtr))
		return false;
	if (labelPtr & 0x3FFFFF) {
		if (labelPtr & 0x400000) {
			quint32 lblOff;
			if (lblOffset(hdl, labelPtr & 0x3FFFFF, lblOff) && lblOff)
				label = lbl->label(lblHdl, lblOff);
		} else
			label = lbl->label(lblHdl, labelPtr & 0x3FFFFF);
	}

	return true;
}

bool NETFile::init(Handle &hdl)
{
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size) && readUInt8(hdl, _shift)))
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

	_init = true;

	return true;
}

bool NETFile::link(const SubDiv *subdiv, quint32 shift, Handle &hdl,
  NODFile *nod, Handle &nodHdl, LBLFile *lbl, Handle &lblHdl,
  const NODFile::BlockInfo &blockInfo, quint8 linkId, quint8 lineId,
  const HuffmanTable &table, QList<IMG::Poly> *lines)
{
	if (!_init && !init(hdl))
		return false;

	Q_ASSERT(_tableId == table.id());
	if (_tableId != table.id())
		return false;

	IMG::Poly poly;
	if (!nod->linkType(nodHdl, blockInfo, linkId, poly.type))
		return false;

	NODFile::LinkInfo linkInfo;
	if (!nod->linkInfo(nodHdl, blockInfo, linkId, linkInfo))
		return false;

	quint32 linkOffset = _linksOffset + (linkInfo.linkOffset << _linksShift);
	if (linkOffset > _linksOffset + _linksSize)
		return false;
	if (!seek(hdl, linkOffset))
		return false;
	BitStream4R bs(*this, hdl, linkOffset - _linksOffset);
	QVector<quint16> ca;
	quint16 mask = 0;
	quint32 size;

	quint8 s68 = (linkInfo.flags >> 0x12) & 1;
	quint8 s69 = (linkInfo.flags >> 0x11) & 1;
	quint8 s6a = (linkInfo.flags >> 0x13) & 1;

	if (s69 == 0 || s6a == 1) {
		if (!bs.readVUInt32(size))
			return false;
	}
	if (s69 == 0) {
		if (!readAdjCounts(bs, ca, mask))
			return false;
	}

	if (!subdiv->level()) {
		NODFile::AdjacencyInfo adj(nod, blockInfo, linkId, linkInfo);

		if (s69 == 1) {
			if (s68 == 1) {
				if (!readShape(nod, nodHdl, adj, bs, table, subdiv, shift, poly))
					return false;
			} else {
				if (!readNodeGeometry(nod, nodHdl, adj, poly))
					return false;
			}
		} else {
			quint16 mask2 = mask + 0xffff;
			for (int i = 0; i <= ca.size(); i++) {
				quint16 step = (i < ca.size()) ? ca.at(i) & mask2 : 0xFFFF;
				bool shape = (i > 0) ? ca.at(i-1) & mask : (s68 == 1);
				if (i == lineId) {
					if (shape) {
						bool check = (i < ca.size()) ? (ca.at(i) & mask) : false;
						if (!readShape(nod, nodHdl, adj, bs, table, subdiv,
						  shift, poly, step, check))
							return false;
					} else {
						if (!readNodeGeometry(nod, nodHdl, adj, poly, step))
							return false;
					}
					break;
				}

				if (shape && !skipShape(bs))
					return false;
				if (!skipNodes(nod, nodHdl, adj, step))
					return false;
			}
		}
	} else {
		if (!skipAdjShapes(bs, ca, mask, s68 == 1))
			return false;

		if (!seekToLevel(bs, subdiv->level()))
			return false;
		if (!seekToLine(bs, lineId))
			return false;
		if (!readLine(bs, subdiv, table, poly))
			return false;
	}

	if (lbl)
		linkLabel(hdl, linkOffset, _linksSize - (linkOffset - _linksOffset),
		  lbl, lblHdl, poly.label);

	lines->append(poly);

	return true;
}

bool NETFile::lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset)
{
	if (!_init && !init(hdl))
		return false;

	if (!(seek(hdl, _offset + (netOffset << _shift))
	  && readUInt24(hdl, lblOffset)))
		return false;

	lblOffset &= 0x3FFFFF;

	return true;
}
