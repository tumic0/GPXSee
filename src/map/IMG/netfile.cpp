#include "bitstream.h"
#include "huffmanstream.h"
#include "subdiv.h"
#include "nodfile.h"
#include "lblfile.h"
#include "rgnfile.h"
#include "netfile.h"

using namespace Garmin;
using namespace IMG;

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

	if (!bs.readVUint32SM(v1, v2, v2b))
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
		if (!bs.readVUint32SM(v1, v2, v2b))
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
		if (!bs.readVUint32SM(v1, v2, v2b))
			return false;
		if (!bs.skip(v1))
			return false;

		if (v2 & 2)
			return false;
	}

	return true;
}

static bool readNodeGeometry(const NODFile *nod, SubFile::Handle &nodHdl,
  NODFile::AdjacencyInfo &adj, quint16 cnt, MapData::Poly &poly)
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

static bool skipNodes(const NODFile *nod, SubFile::Handle &nodHdl,
  NODFile::AdjacencyInfo &adj, int cnt)
{
	for (int i = 0; i < cnt; i++)
		if (nod->nextNode(nodHdl, adj))
			return false;

	return true;
}


bool NETFile::readLine(BitStream4R &bs, const SubDiv *subdiv,
  MapData::Poly &poly) const
{
	quint32 v1, v2, v2b;
	if (!bs.readVUint32SM(v1, v2, v2b))
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

	HuffmanDeltaStreamR stream(bs, *_tp);
	if (!stream.init())
		return false;
	qint32 lonDelta, latDelta;

	while (stream.readNext(lonDelta, latDelta)) {
		if (!(lonDelta | latDelta))
			break;

		pos.rx() += LS(lonDelta, 32-subdiv->bits());
		pos.ry() += LS(latDelta, 32-subdiv->bits());

		Coordinates c(toWGS32(pos.x()), toWGS32(pos.y()));
		poly.points.append(QPointF(c.lon(), c.lat()));
		poly.boundingRect = poly.boundingRect.united(c);
	}

	return stream.atEnd();
}

bool NETFile::readShape(const NODFile *nod, SubFile::Handle &nodHdl,
  NODFile::AdjacencyInfo &adj, BitStream4R &bs, const SubDiv *subdiv,
  quint32 shift, quint16 cnt, bool check, MapData::Poly &poly) const
{
	quint32 v1, v2, v2b;
	if (!bs.readVUint32SM(v1, v2, v2b))
		return false;
	BitStream4R::State state;
	bs.save(state);
	bs.resize(v1);

	quint32 flags;
	if (!bs.read(8, flags))
		return false;
	flags |= (v2 << 8);

	bool hasAdjustBit = flags & (1 << (v2b + 7));
	bool startWithStream = flags & (1 << (v2b + 6));
	bool useEosBit = flags & (1 << (v2b + 5));

	HuffmanDeltaStreamR stream(bs, *_tp);
	if (!stream.init(flags, v2b + 5))
		return false;


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
		if (hasAdjustBit && !stream.read(1, adjustBit))
			return false;

		if (!(lonDelta | latDelta) && !startWithStream && !hasAdjustBit)
			break;

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
					  && bs.readVUint32SM(v1, v2, v2b)))
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

bool NETFile::linkLabel(Handle &hdl, quint32 offset,
  LBLFile *lbl, Handle &lblHdl, Label &label) const
{
	if (!seek(hdl, offset))
		return false;
	BitStream1 bs(*this, hdl, _links.size - (offset - _links.offset));

	quint32 flags, labelPtr;
	if (!bs.read(8, flags))
		return false;
	if (!(flags & 1))
		return true;

	if (!bs.readUInt24(labelPtr))
		return false;
	if (labelPtr & 0x3FFFFF)
		label = lbl->label(lblHdl, labelPtr & 0x3FFFFF);

	return true;
}

bool NETFile::load(Handle &hdl, const RGNFile *rgn, Handle &rgnHdl)
{
	quint16 hdrLen;

	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _base.offset)
	  && readUInt32(hdl, _base.size) && readByte(hdl, &_netShift)))
		return false;

	if (hdrLen >= 0x4C) {
		quint32 info;
		if (!(seek(hdl, _gmpOffset + 0x37) && readUInt32(hdl, info)))
			return false;
		if (!(seek(hdl, _gmpOffset + 0x43) && readUInt32(hdl, _links.offset)
		  && readUInt32(hdl, _links.size) && readByte(hdl, &_linksShift)))
			return false;

		quint8 tableId = ((info >> 2) & 0x0F);
		if (_links.size && (!rgn->huffmanTable() || rgn->huffmanTable()->id()
		  != tableId)) {
			_huffmanTable = new HuffmanTable(tableId);
			if (!_huffmanTable->load(rgn, rgnHdl))
				return false;
		}

		_tp = _huffmanTable ? _huffmanTable : rgn->huffmanTable();
	}

	return true;
}

void NETFile::clear()
{
	delete _huffmanTable;
	_huffmanTable = 0;
}

NETFile::~NETFile()
{
	delete _huffmanTable;
}

bool NETFile::link(const SubDiv *subdiv, quint32 shift, Handle &hdl,
  const NODFile *nod, Handle &nodHdl2, Handle &nodHdl, LBLFile *lbl,
  Handle &lblHdl, const NODFile::BlockInfo &blockInfo, quint8 linkId,
  quint8 lineId, QList<MapData::Poly> *lines) const
{
	MapData::Poly poly;
	if (!nod->linkType(nodHdl, blockInfo, linkId, poly.type))
		return false;

	NODFile::LinkInfo linkInfo;
	if (!nod->linkInfo(nodHdl, blockInfo, linkId, linkInfo))
		return false;

	quint32 linkOffset = _links.offset + (linkInfo.linkOffset << _linksShift);
	if (linkOffset > _links.offset + _links.size)
		return false;
	if (!seek(hdl, linkOffset))
		return false;
	BitStream4R bs(*this, hdl, linkOffset - _links.offset);
	QVector<quint16> ca;
	quint16 mask = 0;
	quint32 size;

	bool firstIsShape = (linkInfo.flags >> 10) & 1;
	bool singleTopology = (linkInfo.flags >> 9) & 1;
	bool hasLevels = (linkInfo.flags >> 11) & 1;

	if (!singleTopology || hasLevels) {
		if (!bs.readVUInt32(size))
			return false;
	}
	if (!singleTopology) {
		if (!readAdjCounts(bs, ca, mask))
			return false;
	}

	if (!subdiv->level()) {
		NODFile::AdjacencyInfo adj(nodHdl2, blockInfo, linkId, linkInfo);

		if (singleTopology) {
			if (firstIsShape) {
				if (!readShape(nod, nodHdl, adj, bs, subdiv, shift, 0xFFFF,
				  false, poly))
					return false;
			} else {
				if (!readNodeGeometry(nod, nodHdl, adj, 0xFFFF, poly))
					return false;
			}
		} else {
			quint16 mask2 = mask + 0xffff;
			for (int i = 0; i <= ca.size(); i++) {
				quint16 step = (i < ca.size()) ? ca.at(i) & mask2 : 0xFFFF;
				bool shape = (i > 0) ? ca.at(i-1) & mask : firstIsShape;
				if (i == lineId) {
					if (shape) {
						bool check = (i < ca.size()) ? (ca.at(i) & mask) : false;
						if (!readShape(nod, nodHdl, adj, bs, subdiv, shift,
						  step, check, poly))
							return false;
					} else {
						if (!readNodeGeometry(nod, nodHdl, adj, step, poly))
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
		Q_ASSERT(hasLevels);

		if (!skipAdjShapes(bs, ca, mask, firstIsShape))
			return false;

		if (!seekToLevel(bs, subdiv->level()))
			return false;
		if (!seekToLine(bs, lineId))
			return false;
		if (!readLine(bs, subdiv, poly))
			return false;
	}

	if (lbl)
		linkLabel(hdl, linkOffset, lbl, lblHdl, poly.label);
	if ((linkInfo.flags >> 3) & 1)
		poly.flags |= MapData::Poly::OneWay;

	lines->append(poly);

	return true;
}

bool NETFile::lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset) const
{
	if (!(seek(hdl, _base.offset + (netOffset << _netShift))
	  && readUInt24(hdl, lblOffset)))
		return false;

	lblOffset &= 0x3FFFFF;

	return true;
}
