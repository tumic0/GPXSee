#include "common/garmin.h"
#include "subdiv.h"
#include "trefile.h"

using namespace Garmin;
using namespace IMG;

static inline double RB(qint32 val)
{
	return (val == -0x800000 || val >= 0x800000) ? 180.0 : toWGS24(val);
}

static inline double LB(qint32 val)
{
	return (val <= -0x800000) ? -180.0 : toWGS24(val);
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

bool TREFile::init(QFile *file)
{
	Handle hdl(this, file);
	quint8 locked, levels[64];
	quint16 hdrLen;
	qint32 north, east, south, west;
	quint32 levelsCount;
	Section levelSec;


	if (!(seek(hdl, _gmpOffset) && readUInt16(hdl, hdrLen)
	  && seek(hdl, _gmpOffset + 0x0D) && readByte(hdl, &locked)))
		return false;

	// Tile bounds
	if (!(seek(hdl, _gmpOffset + 0x15) && readInt24(hdl, north)
	  && readInt24(hdl, east) && readInt24(hdl, south) && readInt24(hdl, west)))
		return false;
	_bounds = RectC(Coordinates(toWGS24(west), toWGS24(north)),
	  Coordinates(RB(east), toWGS24(south)));
	if (!_bounds.isValid())
		return false;

	// Levels & subdivs info
	if (!(readUInt32(hdl, levelSec.offset) && readUInt32(hdl, levelSec.size)
	  && readUInt32(hdl, _subdivSec.offset) && readUInt32(hdl, _subdivSec.size)))
		return false;

	// Extended objects (TRE7) info
	if (hdrLen > 0x9A) {
		if (!(seek(hdl, _gmpOffset + 0x7C) && readUInt32(hdl, _extSec.offset)
		  && readUInt32(hdl, _extSec.size) && readUInt16(hdl, _extItemSize)
		  && readUInt32(hdl, _flags)))
			return false;
	}

	// Tile levels
	if (levelSec.size > 64)
		return false;
	if (!(seek(hdl, levelSec.offset) && read(hdl, (char*)levels, levelSec.size)))
		return false;

	if (locked) {
		quint32 key;

		if (hdrLen < 0xAE)
			return false;
		if (!(seek(hdl, _gmpOffset + 0xAA) && readUInt32(hdl, key)))
			return false;
		demangle(levels, levelSec.size, key);
	}

	levelsCount = levelSec.size / 4;
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

bool TREFile::load(QFile *file, int idx)
{
	Handle hdl(this, file);
	QList<SubDiv*> sl;
	SubDiv *s = 0;
	SubDivTree *tree = new SubDivTree();
	const MapLevel &level = _levels.at(idx);

	_subdivs.insert(level.level, tree);

	quint32 skip = 0;
	for (int i = 0; i < idx; i++)
		skip += _levels.at(i).subdivs;

	if (!seek(hdl, _subdivSec.offset + skip * 16))
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
		height &= 0x7FFF;
		height = LS(height, 24 - level.bits);

		s = new SubDiv(offset, lon, lat, level.level, level.bits, objects);
		sl.append(s);

		double min[2], max[2];
		RectC bounds(Coordinates(LB(lon - width), toWGS24(lat + height)),
		  Coordinates(RB(lon + width), toWGS24(lat - height)));

		min[0] = bounds.left();
		min[1] = bounds.bottom();
		max[0] = bounds.right();
		max[1] = bounds.top();

		/* both mkgmap and cGPSmapper generate all kinds of broken subdiv bounds
		   (zero lat/lon, zero width/height, ...) so we check only that the
		   subdiv item does not break the rtree, not for full bounds validity. */
		if (!(min[0] <= max[0] && min[1] <= max[1]))
			goto error;

		tree->Insert(min, max, s);
	}

	if (idx != _levels.size() - 1) {
		quint32 offset;
		if (!readUInt24(hdl, offset))
			goto error;
		s->setEnd(offset);
	}


	// Objects with extended types (TRE7)
	if (_extSec.size && _extItemSize) {
		quint32 totalSubdivs = 0;
		for (int i = 0; i < _levels.size(); i++)
			totalSubdivs += _levels.at(i).subdivs;
		quint32 extendedSubdivs = _extSec.size / _extItemSize;
		quint32 diff = totalSubdivs - extendedSubdivs + 1;
		if (skip < diff)
			return true;

		if (!seek(hdl, _extSec.offset + (skip - diff) * _extItemSize))
			goto error;

		quint32 polygons, lines, points;
		int rb;
		for (int i = 0; i < sl.size(); i++) {
			if ((rb = readExtEntry(hdl, polygons, lines, points)) < 0)
				goto error;

			sl.at(i)->setExtOffsets(polygons, lines, points);
			if (i)
				sl.at(i-1)->setExtEnds(polygons, lines, points);

			if (!seek(hdl, pos(hdl) + _extItemSize - rb))
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

const TREFile::SubDivTree *TREFile::subdivs(QFile *file, const Zoom &zoom)
{
	int idx = -1;

	for (int i = _firstLevel; i < _levels.size(); i++) {
		if (_levels.at(i).level == zoom.level()
		  && _levels.at(i).bits == zoom.bits()) {
			idx = i;
			break;
		}
	}
	if (idx < 0)
		return 0;

	if (!_subdivs.contains(_levels.at(idx).level) && !load(file, idx))
		return 0;

	return _subdivs.value(_levels.at(idx).level);
}

static bool cb(SubDiv *subdiv, void *context)
{
	QList<SubDiv*> *list = (QList<SubDiv*>*)context;
	list->append(subdiv);
	return true;
}

QList<SubDiv*> TREFile::subdivs(QFile *file, const RectC &rect, const Zoom &zoom)
{
	QList<SubDiv*> list;
	const SubDivTree *tree = subdivs(file, zoom);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	if (tree)
		tree->Search(min, max, cb, &list);

	return list;
}

QVector<Zoom> TREFile::zooms() const
{
	QVector<Zoom> ret;

	for (int i = _firstLevel; i < _levels.size(); i++)
		ret.append(Zoom(_levels.at(i).level, _levels.at(i).bits));

	return ret;
}
