#include "subdiv.h"
#include "units.h"
#include "trefile.h"


static void unlock(quint8 *dst, const quint8 *src, quint32 size, quint32 key)
{
	static const unsigned char shuf[] = {
		0xb, 0xc, 0xa, 0x0,
		0x8, 0xf, 0x2, 0x1,
		0x6, 0x4, 0x9, 0x3,
		0xd, 0x5, 0x7, 0xe
	};

	int sum = shuf[((key >> 24) + (key >> 16) + (key >> 8) + key) & 0xf];

	for (quint32 i = 0, ringctr = 16; i < size; i++) {
		quint32 upper = src[i] >> 4;
		quint32 lower = src[i];

		upper -= sum;
		upper -= key >> ringctr;
		upper -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		lower -= sum;
		lower -= key >> ringctr;
		lower -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		dst[i] = ((upper << 4) & 0xf0) | (lower & 0xf);
	}
}

TREFile::~TREFile()
{
	clear();
}

bool TREFile::init()
{
	Handle hdl;
	quint8 locked;
	quint16 hdrLen;

	if (!(seek(hdl, 0) && readUInt16(hdl, hdrLen)
	  && seek(hdl, 0x0D) && readByte(hdl, locked)))
		return false;

	// Tile bounds
	qint32 north, east, south, west;
	if (!(seek(hdl, 0x15) && readInt24(hdl, north) && readInt24(hdl, east)
	  && readInt24(hdl, south) && readInt24(hdl, west)))
		return false;
	_bounds = RectC(Coordinates(toWGS84(west), toWGS84(north)),
	  Coordinates(toWGS84(east), toWGS84(south)));

	// Levels & subdivs info
	quint32 levelsOffset, levelsSize, subdivSize;
	if (!(seek(hdl, 0x21) && readUInt32(hdl, levelsOffset)
	  && readUInt32(hdl, levelsSize) && readUInt32(hdl, _subdivOffset)
	  && readUInt32(hdl, subdivSize)))
		return false;

	// TRE7 info
	if (hdrLen > 0x9A) {
		if (!(seek(hdl, 0x7C) && readUInt32(hdl, _extended.offset)
		  && readUInt32(hdl, _extended.size)
		  && readUInt16(hdl, _extended.itemSize)))
			return false;
	}

	// Tile levels
	if (levelsSize > 64 || !seek(hdl, levelsOffset))
		return false;
	quint8 levels[64];
	for (quint32 i = 0; i < levelsSize; i++)
		if (!readByte(hdl, levels[i]))
			return false;
	if (locked) {
		quint32 key;
		quint8 unlocked[64];
		if (!seek(hdl, 0xAA) || !readUInt32(hdl, key))
			return false;
		unlock(unlocked, levels, levelsSize, key);
		memcpy(levels, unlocked, levelsSize);
	}

	quint32 levelsCount = levelsSize / 4;
	_levels = QVector<MapLevel>(levelsCount);

	for (quint32 i = 0; i < levelsCount; i++) {
		quint8 *zoom = levels + (i * 4);
		_levels[i].level = *zoom;
		_levels[i].bits = *(zoom + 1);
		_levels[i].subdivs = *(zoom + 2) | (quint16)(*(zoom + 3)) << 8;
		if (_levels[i].bits > 24 || !_levels[i].subdivs)
			return false;
	}

	// Get first non-inherited level
	_firstLevel = -1;
	for (int i = 0; i < _levels.size(); i++) {
		if (!(_levels.at(i).level & 0x80)) {
			_firstLevel = i;
			break;
		}
	}

	return (_firstLevel >= 0);
}

bool TREFile::load(int idx)
{
	Handle hdl;
	QList<SubDiv*> sl;
	SubDiv *s = 0;
	SubDivTree *tree = new SubDivTree();


	_subdivs.insert(_levels.at(idx).bits, tree);

	quint32 skip = 0;
	for (int i = 0; i < idx; i++)
		skip += _levels.at(i).subdivs;

	if (!seek(hdl, _subdivOffset + skip * 16))
		return false;

	for (int j = 0; j < _levels.at(idx).subdivs; j++) {
		quint32 offset;
		qint32 lon, lat;
		quint8 objects;
		quint16 width, height, nextLevel;

		if (!(readUInt24(hdl, offset) && readByte(hdl, objects)
		  && readInt24(hdl, lon) && readInt24(hdl, lat)
		  && readUInt16(hdl, width) && readUInt16(hdl, height)))
			goto error;
		if (idx != _levels.size() - 1)
			if (!readUInt16(hdl, nextLevel))
				goto error;

		if (s)
			s->setEnd(offset);

		width &= 0x7FFF;
		width <<= (24 - _levels.at(idx).bits);
		height <<= (24 - _levels.at(idx).bits);

		s = new SubDiv(offset, lon, lat, _levels.at(idx).bits, objects);
		sl.append(s);

		double min[2], max[2];
		RectC bounds(Coordinates(toWGS84(lon - width),
		  toWGS84(lat + height + 1)), Coordinates(toWGS84(lon + width + 1),
		  toWGS84(lat - height)));

		min[0] = bounds.left();
		min[1] = bounds.bottom();
		max[0] = bounds.right();
		max[1] = bounds.top();

		tree->Insert(min, max, s);
	}

	if (idx != _levels.size() - 1) {
		quint32 offset;
		if (!readUInt24(hdl, offset))
			goto error;
		s->setEnd(offset);
	}


	// Objects with extended types (TRE7)
	if (_extended.size && _extended.itemSize >= 12) {
		/* Some maps skip entries for the inherited levels, some don't. Our
		   decision is based on the difference between the extended subdivs
		   count and the total subdivs count. */
		quint32 totalSubdivs = 0;
		for (int i = 0; i < _levels.size(); i++)
			totalSubdivs += _levels.at(i).subdivs;
		quint32 extendedSubdivs = _extended.size / _extended.itemSize;
		quint32 diff = totalSubdivs - extendedSubdivs + 1;

		quint32 polygons, lines, points;
		if (!seek(hdl, _extended.offset + (skip - diff) * _extended.itemSize))
			goto error;

		for (int i = 0; i < sl.size(); i++) {
			if (!(readUInt32(hdl, polygons) && readUInt32(hdl, lines)
			  && readUInt32(hdl, points)))
				goto error;

			sl.at(i)->setExtOffsets(polygons, lines, points);
			if (i)
				sl.at(i-1)->setExtEnds(polygons, lines, points);

			if (!seek(hdl, hdl.pos + _extended.itemSize - 12))
				goto error;
		}

		if (idx != _levels.size() - 1) {
			if (!(readUInt32(hdl, polygons) && readUInt32(hdl, lines)
			  && readUInt32(hdl, points)))
				goto error;
			sl.last()->setExtEnds(polygons, lines, points);
		}
	}

	return true;

error:
	qDeleteAll(sl);
	tree->RemoveAll();

	return false;
}

void TREFile::clear()
{
	SubDivTree::Iterator jt;

	for (QMap<int, SubDivTree*>::iterator it = _subdivs.begin();
	  it != _subdivs.end(); ++it) {
		SubDivTree *tree = *it;
		for (tree->GetFirst(jt); !tree->IsNull(jt); tree->GetNext(jt))
			delete tree->GetAt(jt);
	}

	qDeleteAll(_subdivs);

	_subdivs.clear();
}

int TREFile::level(int bits)
{
	int idx = _firstLevel;

	for (int i = idx + 1; i < _levels.size(); i++) {
		if (_levels.at(i).bits > bits)
			break;
		idx++;
	}

	if (!_subdivs.contains(_levels.at(idx).bits) && !load(idx))
		return -1;

	return _levels.at(idx).bits;
}

static bool cb(SubDiv *subdiv, void *context)
{
	QList<SubDiv*> *list = (QList<SubDiv*>*)context;
	list->append(subdiv);
	return true;
}

QList<SubDiv*> TREFile::subdivs(const RectC &rect, int bits)
{
	QList<SubDiv*> list;
	SubDivTree *tree = _subdivs.value(level(bits));
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	if (tree)
		tree->Search(min, max, cb, &list);

	return list;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const TREFile::MapLevel &level)
{
	dbg.nospace() << "MapLevel(" << level.level << ", " << level.bits << ", "
	  << level.subdivs << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
