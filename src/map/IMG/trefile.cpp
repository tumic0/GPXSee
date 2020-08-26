#include "common/garmin.h"
#include "subdiv.h"
#include "trefile.h"


static inline double RB(qint32 val)
{
	return (val == -0x800000 || val == 0x800000) ? 180.0 : toWGS24(val);
}

static void demangle(quint8 *data, quint32 size, quint32 key)
{
	static const unsigned char shuf[] = {
		0xb, 0xc, 0xa, 0x0,
		0x8, 0xf, 0x2, 0x1,
		0x6, 0x4, 0x9, 0x3,
		0xd, 0x5, 0x7, 0xe
	};

	int sum = shuf[((key >> 24) + (key >> 16) + (key >> 8) + key) & 0xf];

	for (quint32 i = 0, ringctr = 16; i < size; i++) {
		quint32 upper = data[i] >> 4;
		quint32 lower = data[i];

		upper -= sum;
		upper -= key >> ringctr;
		upper -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		lower -= sum;
		lower -= key >> ringctr;
		lower -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		data[i] = ((upper << 4) & 0xf0) | (lower & 0xf);
	}
}

TREFile::~TREFile()
{
	clear();
}

bool TREFile::init()
{
	Handle hdl(this);
	quint8 locked;
	quint16 hdrLen;


	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x0D) && readUInt8(hdl, locked)))
		return false;

	// Tile bounds
	qint32 north, east, south, west;
	if (!(seek(hdl, _gmpOffset + 0x15) && readInt24(hdl, north)
	  && readInt24(hdl, east) && readInt24(hdl, south) && readInt24(hdl, west)))
		return false;
	_bounds = RectC(Coordinates(toWGS24(west), toWGS24(north)),
	  Coordinates(RB(east), toWGS24(south)));
	Q_ASSERT(_bounds.left() <= _bounds.right());

	// Levels & subdivs info
	quint32 levelsOffset, levelsSize, subdivSize;
	if (!(seek(hdl, _gmpOffset + 0x21) && readUInt32(hdl, levelsOffset)
	  && readUInt32(hdl, levelsSize) && readUInt32(hdl, _subdivOffset)
	  && readUInt32(hdl, subdivSize)))
		return false;

	if (hdrLen > 0x9A) {
		// TRE7 info + flags
		if (!(seek(hdl, _gmpOffset + 0x7C) && readUInt32(hdl, _extended.offset)
		  && readUInt32(hdl, _extended.size)
		  && readUInt16(hdl, _extended.itemSize) && readUInt32(hdl, _flags)))
			return false;
	} else {
		_extended.offset = 0;
		_extended.size = 0;
		_extended.itemSize = 0;
		_flags = 0;
	}

	// Tile levels
	if (levelsSize > 64 || !seek(hdl, levelsOffset))
		return false;
	quint8 levels[64];
	for (quint32 i = 0; i < levelsSize; i++)
		if (!readUInt8(hdl, levels[i]))
			return false;
	if (locked) {
		quint32 key;
		if (!seek(hdl, _gmpOffset + 0xAA) || !readUInt32(hdl, key))
			return false;
		demangle(levels, levelsSize, key);
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

	_isBaseMap = false;

	return (_firstLevel >= 0);
}

int TREFile::readExtEntry(Handle &hdl, quint32 &polygons, quint32 &lines,
  quint32 &points)
{
	int rb = 0;

	if (_flags & 1) {
		if (!readUInt32(hdl, polygons))
			return -1;
		rb += 4;
	} else
		polygons = 0;
	if (_flags & 2) {
		if (!readUInt32(hdl, lines))
			return -1;
		rb += 4;
	} else
		lines = 0;
	if (_flags & 4) {
		if (!readUInt32(hdl, points))
			return -1;
		rb += 4;
	} else
		points = 0;

	return rb;
}

bool TREFile::load(int idx)
{
	Handle hdl(this);
	QList<SubDiv*> sl;
	SubDiv *s = 0;
	SubDivTree *tree = new SubDivTree();
	const MapLevel &level = _levels.at(idx);


	_subdivs.insert(level.bits, tree);

	quint32 skip = 0;
	for (int i = 0; i < idx; i++)
		skip += _levels.at(i).subdivs;

	if (!seek(hdl, _subdivOffset + skip * 16))
		return false;

	for (int j = 0; j < level.subdivs; j++) {
		quint32 oo;
		qint32 lon, lat, width, height;
		quint16 nextLevel;

		if (!(readUInt32(hdl, oo) && readInt24(hdl, lon) && readInt24(hdl, lat)
		  && readUInt16(hdl, width) && readUInt16(hdl, height)))
			goto error;
		quint32 offset = oo & 0xfffffff;
		quint8 objects = (((qint16)height < 0) << 4) | (oo >> 0x1c);

		if (idx != _levels.size() - 1)
			if (!readUInt16(hdl, nextLevel))
				goto error;

		if (s)
			s->setEnd(offset);

		width &= 0x7FFF;
		width = LS(width, 24 - level.bits);
		height = LS(height, 24 - level.bits);

		s = new SubDiv(offset, lon, lat, level.level, level.bits, objects);
		sl.append(s);

		double min[2], max[2];
		RectC bounds(Coordinates(toWGS24(lon - width), toWGS24(lat + height)),
		  Coordinates(RB(lon + width), toWGS24(lat - height)));
		Q_ASSERT(bounds.left() <= bounds.right());

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
	if (_extended.size && _extended.itemSize) {
		quint32 totalSubdivs = 0;
		for (int i = 0; i < _levels.size(); i++)
			totalSubdivs += _levels.at(i).subdivs;
		quint32 extendedSubdivs = _extended.size / _extended.itemSize;
		quint32 diff = totalSubdivs - extendedSubdivs + 1;
		if (!seek(hdl, _extended.offset + (skip - diff) * _extended.itemSize))
			goto error;

		quint32 polygons, lines, points;
		int rb;
		for (int i = 0; i < sl.size(); i++) {
			if ((rb = readExtEntry(hdl, polygons, lines, points)) < 0)
				goto error;

			sl.at(i)->setExtOffsets(polygons, lines, points);
			if (i)
				sl.at(i-1)->setExtEnds(polygons, lines, points);

			if (!seek(hdl, pos(hdl) + _extended.itemSize - rb))
				goto error;
		}

		if (idx != _levels.size() - 1) {
			if (readExtEntry(hdl, polygons, lines, points) < 0)
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

int TREFile::level(int bits, bool baseMap)
{
	if (baseMap) {
		if (!_isBaseMap && _levels.first().bits > bits)
			return -1;
		if (_isBaseMap && bits > _levels.last().bits)
			return -1;
	}

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

QList<SubDiv*> TREFile::subdivs(const RectC &rect, int bits, bool baseMap)
{
	QList<SubDiv*> list;
	SubDivTree *tree = _subdivs.value(level(bits, baseMap));
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
