#include <cstring>
#include <QtEndian>
#include <QFile>
#include <QDataStream>
#include <QColor>
#include "map/osm.h"
#include "subfile.h"
#include "mapdata.h"

using namespace Mapsforge;

#define MAGIC "mapsforge binary OSM"
#define MAGIC_SIZE (sizeof(MAGIC) - 1)
#define MD(val) ((val) / 1e6)
#define OFFSET_MASK 0x7FFFFFFFFFL

#define KEY_NAME  "name"
#define KEY_HOUSE "addr:housenumber"
#define KEY_REF   "ref"
#define KEY_ELE   "ele"

static void copyPaths(const RectC &rect, const QList<MapData::Path> *src,
  QList<MapData::Path> *dst)
{
	for (int i = 0; i < src->size(); i++) {
		const MapData::Path &path = src->at(i);
		if (rect.intersects(path.poly.boundingRect()))
			dst->append(path);
	}
}

static void copyPoints(const RectC &rect, const QList<MapData::Point> *src,
  QList<MapData::Point> *dst)
{
	for (int i = 0; i < src->size(); i++) {
		const MapData::Point &point = src->at(i);
		if (rect.contains(point.coordinates))
			dst->append(point);
	}
}

static void copyPoints(const RectC &rect, const QList<MapData::Path> *src,
  QList<MapData::Point> *dst)
{
	for (int i = 0; i < src->size(); i++) {
		const MapData::Path &path = src->at(i);
		if (path.closed && rect.contains(path.point.coordinates))
			dst->append(path.point);
	}
}

static double distance(const Coordinates &c1, const Coordinates &c2)
{
	return hypot(c1.lon() - c2.lon(), c1.lat() - c2.lat());
}

static bool isClosed(const QVector<Coordinates> &poly)
{
	return (distance(poly.first(), poly.last()) < 0.000000001);
}

static bool readSingleDelta(SubFile &subfile, const Coordinates &c,
  int count, QVector<Coordinates> &nodes)
{
	qint32 mdLat, mdLon;

	if (!(subfile.readVInt32(mdLat) && subfile.readVInt32(mdLon)))
		return false;

	double lat = c.lat() + MD(mdLat);
	double lon = c.lon() + MD(mdLon);

	nodes.reserve(count);
	nodes.append(Coordinates(lon, lat));

	for (int i = 1; i < count; i++) {
		if (!(subfile.readVInt32(mdLat) && subfile.readVInt32(mdLon)))
			return false;

		lat = lat + MD(mdLat);
		lon = lon + MD(mdLon);

		nodes.append(Coordinates(lon, lat));
	}

	return true;
}

static bool readDoubleDelta(SubFile &subfile, const Coordinates &c,
  int count, QVector<Coordinates> &nodes)
{
	qint32 mdLat, mdLon;

	if (!(subfile.readVInt32(mdLat) && subfile.readVInt32(mdLon)))
		return false;

	double lat = c.lat() + MD(mdLat);
	double lon = c.lon() + MD(mdLon);
	double prevLat = 0;
	double prevLon = 0;

	nodes.reserve(count);
	nodes.append(Coordinates(lon, lat));

	for (int i = 1; i < count; i++) {
		if (!(subfile.readVInt32(mdLat) && subfile.readVInt32(mdLon)))
			return false;

		double singleLat = MD(mdLat) + prevLat;
		double singleLon = MD(mdLon) + prevLon;

		lat += singleLat;
		lon += singleLon;

		nodes.append(Coordinates(lon, lat));

		prevLat = singleLat;
		prevLon = singleLon;
	}

	return true;
}

static bool readPolygonPath(SubFile &subfile, const Coordinates &c,
  bool doubleDelta, Polygon &polygon)
{
	quint32 blocks, nodes;

	if (!subfile.readVUInt32(blocks))
		return false;

	polygon.reserve(polygon.size() + blocks);
	for (quint32 i = 0; i < blocks; i++) {
		if (!subfile.readVUInt32(nodes) || !nodes)
			return false;

		QVector<Coordinates> path;

		if (doubleDelta) {
			if (!readDoubleDelta(subfile, c, nodes, path))
				return false;
		} else {
			if (!readSingleDelta(subfile, c, nodes, path))
				return false;
		}

		polygon.append(path);
	}

	return true;
}

static bool readOffset(QDataStream &stream, quint64 &offset)
{
	quint8 b0, b1, b2, b3, b4;

	stream >> b0 >> b1 >> b2 >> b3 >> b4;
	offset = b4 | ((quint64)b3) << 8 | ((quint64)b2) << 16
	  | ((quint64)b1) << 24 | ((quint64)b0) << 32;

	return (stream.status() == QDataStream::Ok);
}

bool MapData::readTags(SubFile &subfile, int count,
  const QVector<TagSource> &tags, QVector<Tag> &list)
{
	QVector<quint32> ids(count);

	list.resize(count);

	for (int i = 0; i < count; i++) {
		if (!subfile.readVUInt32(ids[i]))
			return false;
		if (ids[i] >= (quint32)tags.size())
			return false;
	}

	for (int i = 0; i < count; i++) {
		const TagSource &tag = tags.at(ids.at(i));

		if (tag.value.length() == 2 && tag.value.at(0) == '%') {
			QByteArray value;

			if (tag.value.at(1) == 'b') {
				quint8 b;
				if (!subfile.readByte(b))
					return false;
				value.setNum(b);
			} else if (tag.value.at(1) == 'i') {
				qint32 u;
				if (!subfile.readInt32(u))
					return false;
				if (tag.key.contains(":colour"))
					value = QColor((quint32)u).name().toLatin1();
				else
					value.setNum(u);
			} else if (tag.value.at(1) == 'f') {
				quint32 u;
				if (!subfile.readUInt32(u))
					return false;
				float *f = (float *)&u;
				value.setNum(*f);
			} else if (tag.value.at(1) == 'h') {
				quint16 s;
				if (!subfile.readUInt16(s))
					return false;
				value.setNum(s);
			} else if (tag.value.at(1) == 's') {
				if (!subfile.readString(value))
					return false;
			} else
				value = tag.value;

			list[i] = MapData::Tag(tag.id, value);
		} else
			list[i] = MapData::Tag(tag.id, tag.value);
	}

	return true;
}

bool MapData::readSubFiles(QFile &file)
{
	QDataStream stream(&file);

	for (int i = 0; i < _subFiles.size(); i++) {
		const SubFileInfo &f = _subFiles.at(i);
		quint64 offset, nextOffset;

		if (!stream.device()->seek(f.offset))
			return false;

		QPoint tl(OSM::ll2tile(_bounds.topLeft(), f.base));
		QPoint br(OSM::ll2tile(_bounds.bottomRight(), f.base));

		if (!readOffset(stream, offset) || (offset & OFFSET_MASK) > f.size)
			return false;

		_tiles.append(new TileTree());

		for (int h = tl.y(); h <= br.y(); h++) {
			for (int w = tl.x(); w <= br.x(); w++) {
				if (!(h == br.y() && w == br.x())) {
					if (!readOffset(stream, nextOffset)
					  || (nextOffset & OFFSET_MASK) > f.size)
						return false;
					if (nextOffset == offset)
						continue;
				}

				Coordinates ttl(OSM::tile2ll(QPoint(w, h), f.base));
				ttl = Coordinates(ttl.lon(), -ttl.lat());
				Coordinates tbr(OSM::tile2ll(QPoint(w + 1, h + 1), f.base));
				tbr = Coordinates(tbr.lon(), -tbr.lat());
				RectC bounds(ttl, tbr);

				double min[2], max[2];
				min[0] = bounds.left();
				min[1] = bounds.bottom();
				max[0] = bounds.right();
				max[1] = bounds.top();
				_tiles.last()->Insert(min, max, new VectorTile(offset, bounds));

				offset = nextOffset;
			}
		}
	}

	return true;
}

bool MapData::readZoomInfo(SubFile &hdr)
{
	quint8 zooms;

	if (!hdr.readByte(zooms))
		return false;

	_subFiles.resize(zooms);
	for (quint8 i = 0; i < zooms; i++) {
		if (!(hdr.readByte(_subFiles[i].base)
		  && hdr.readByte(_subFiles[i].min)
		  && hdr.readByte(_subFiles[i].max)
		  && hdr.readUInt64(_subFiles[i].offset)
		  && hdr.readUInt64(_subFiles[i].size)))
			return false;
	}

	return true;
}

bool MapData::readTagInfo(SubFile &hdr, QVector<TagSource> &tags)
{
	quint16 size;
	QByteArray str;

	if (!hdr.readUInt16(size))
		return false;
	tags.resize(size);

	for (quint16 i = 0; i < size; i++) {
		TagSource &tag = tags[i];
		if (!hdr.readString(str))
			return false;
		tag = str;
		unsigned key = _keys.value(tag.key);
		if (key)
			tag.id = key;
		else {
			tag.id = _keys.size() + 1;
			_keys.insert(tag.key, tag.id);
		}
	}

	return true;
}

bool MapData::readTagInfo(SubFile &hdr)
{
	_keys.insert(KEY_NAME, ID_NAME);
	_keys.insert(KEY_HOUSE, ID_HOUSE);
	_keys.insert(KEY_REF, ID_REF);
	_keys.insert(KEY_ELE, ID_ELE);

	return (readTagInfo(hdr, _pointTags) && readTagInfo(hdr, _pathTags));
}

bool MapData::readMapInfo(SubFile &hdr, QByteArray &projection, bool &debugMap)
{
	quint64 fileSize, date;
	quint32 version;
	qint32 minLat, minLon, maxLat, maxLon;
	quint8 flags;

	if (!(hdr.seek(4) && hdr.readUInt32(version) && hdr.readUInt64(fileSize)
	  && hdr.readUInt64(date) && hdr.readInt32(minLat) && hdr.readInt32(minLon)
	  && hdr.readInt32(maxLat) && hdr.readInt32(maxLon)
	  && hdr.readUInt16(_tileSize) && hdr.readString(projection)
	  && hdr.readByte(flags)))
		return false;

	if (flags & 0x40) {
		qint32 startLon, startLat;
		if (!(hdr.readInt32(startLat) && hdr.readInt32(startLon)))
			return false;
	}
	if (flags & 0x20) {
		quint8 startZoom;
		if (!hdr.readByte(startZoom))
			return false;
	}
	if (flags & 0x10) {
		QByteArray lang;
		if (!hdr.readString(lang))
			return false;
	}
	if (flags & 0x08) {
		QByteArray comment;
		if (!hdr.readString(comment))
			return false;
	}
	if (flags & 0x04) {
		QByteArray createdBy;
		if (!hdr.readString(createdBy))
			return false;
	}

	_bounds = RectC(Coordinates(MD(minLon), MD(maxLat)),
	  Coordinates(MD(maxLon), MD(minLat)));
	_bounds &= OSM::BOUNDS;
	debugMap = flags & 0x80;

	return true;
}

bool MapData::readHeader(QFile &file)
{
	char magic[MAGIC_SIZE];
	quint32 hdrSize;
	QByteArray projection;
	bool debugMap;


	if (file.read(magic, MAGIC_SIZE) < (qint64)MAGIC_SIZE
	  || memcmp(magic, MAGIC, MAGIC_SIZE)) {
		_errorString = "Not a Mapsforge map";
		return false;
	}

	if (file.read((char*)&hdrSize, sizeof(hdrSize)) < (qint64)sizeof(hdrSize)) {
		_errorString = "Unexpected EOF";
		return false;
	}

	SubFile hdr(file, MAGIC_SIZE, qFromBigEndian(hdrSize));

	if (!readMapInfo(hdr, projection, debugMap)) {
		_errorString = "Error reading map info";
		return false;
	}

	if (!readTagInfo(hdr)) {
		_errorString = "Error reading tags info";
		return false;
	}

	if (!readZoomInfo(hdr)) {
		_errorString = "Error reading zooms info";
		return false;
	}

	if (projection != "Mercator") {
		_errorString = projection + ": invalid/unsupported projection";
		return false;
	}
	if (debugMap) {
		_errorString = "DEBUG maps not supported";
		return false;
	}

	return true;
}

MapData::MapData(const QString &fileName) : _fileName(fileName), _valid(false)
{
	QFile file(fileName);

	if (!file.open(QFile::ReadOnly | QIODevice::Unbuffered)) {
		_errorString = file.errorString();
		return;
	}

	if (!readHeader(file))
		return;

	_pathCache.setMaxCost(2048);
	_pointCache.setMaxCost(2048);

	_valid = true;
}

MapData::~MapData()
{
	clearTiles();
}

RectC MapData::bounds() const
{
	/* Align the map bounds with the OSM tiles to prevent area overlap artifacts
	   at least when using EPSG:3857 projection. */
	int zoom = _subFiles.last().base;
	QPoint tl(OSM::mercator2tile(OSM::ll2m(_bounds.topLeft()), zoom));
	Coordinates ctl(OSM::tile2ll(tl, zoom));
	ctl.rlat() = -ctl.lat();

	return RectC(ctl, _bounds.bottomRight());
}

void MapData::load()
{
	QFile file(_fileName);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered))
		qWarning("%s: %s", qPrintable(file.fileName()),
		  qPrintable(file.errorString()));
	else
		readSubFiles(file);
}

void MapData::clear()
{
	_pathCache.clear();
	_pointCache.clear();

	clearTiles();
}

void MapData::clearTiles()
{
	TileTree::Iterator it;

	for (int i = 0; i < _tiles.size(); i++) {
		TileTree *t = _tiles.at(i);
		for (t->GetFirst(it); !t->IsNull(it); t->GetNext(it))
			delete t->GetAt(it);
	}

	qDeleteAll(_tiles);
	_tiles = QList<TileTree*>();
}

bool MapData::pathCb(VectorTile *tile, void *context)
{
	PathCTX *ctx = (PathCTX*)context;
	ctx->data->paths(ctx->file, tile, ctx->rect, ctx->zoom, ctx->list);
	return true;
}

bool MapData::pointCb(VectorTile *tile, void *context)
{
	PointCTX *ctx = (PointCTX*)context;
	ctx->data->points(ctx->file, tile, ctx->rect, ctx->zoom, ctx->list);
	return true;
}

int MapData::level(int zoom) const
{
	for (int i = 0; i < _subFiles.size(); i++)
		if (zoom <= _subFiles.at(i).max)
			return i;

	return _subFiles.size() - 1;
}

void MapData::points(QFile &file, const RectC &rect, int zoom,
  QList<Point> *list)
{
	if (!rect.isValid())
		return;

	int l(level(zoom));
	PointCTX ctx(file, this, rect, zoom, list);
	double min[2], max[2];

	min[0] = rect.left();
	min[1] = rect.bottom();
	max[0] = rect.right();
	max[1] = rect.top();

	_tiles.at(l)->Search(min, max, pointCb, &ctx);
}

void MapData::points(QFile &file, VectorTile *tile, const RectC &rect,
  int zoom, QList<Point> *list)
{
	Key key(tile, zoom);

	tile->lock.lock();

	_pointCacheLock.lock();
	QList<Point> *tilePoints = _pointCache.object(key);
	if (!tilePoints) {
		_pointCacheLock.unlock();
		QList<Point> *p = new QList<Point>();
		if (readPoints(file, tile, zoom, p)) {
			copyPoints(rect, p, list);
			_pointCacheLock.lock();
			_pointCache.insert(key, p);
			_pointCacheLock.unlock();
		} else
			delete p;
	} else {
		copyPoints(rect, tilePoints, list);
		_pointCacheLock.unlock();
	}

	_pathCacheLock.lock();
	QList<Path> *tilePaths = _pathCache.object(key);
	if (!tilePaths) {
		_pathCacheLock.unlock();
		QList<Path> *p = new QList<Path>();
		if (readPaths(file, tile, zoom, p)) {
			copyPoints(rect, p, list);
			_pathCacheLock.lock();
			_pathCache.insert(key, p);
			_pathCacheLock.unlock();
		} else
			delete p;
	} else {
		copyPoints(rect, tilePaths, list);
		_pathCacheLock.unlock();
	}

	tile->lock.unlock();
}

void MapData::paths(QFile &file, const RectC &searchRect,
  const RectC &boundsRect, int zoom, QList<Path> *list)
{
	if (!searchRect.isValid())
		return;

	int l(level(zoom));
	PathCTX ctx(file, this, boundsRect, zoom, list);
	double min[2], max[2];

	min[0] = searchRect.left();
	min[1] = searchRect.bottom();
	max[0] = searchRect.right();
	max[1] = searchRect.top();

	_tiles.at(l)->Search(min, max, pathCb, &ctx);
}

void MapData::paths(QFile &file, VectorTile *tile, const RectC &rect, int zoom,
  QList<Path> *list)
{
	Key key(tile, zoom);

	tile->lock.lock();

	_pathCacheLock.lock();
	QList<Path> *cached = _pathCache.object(key);
	if (!cached) {
		_pathCacheLock.unlock();
		QList<Path> *p = new QList<Path>();
		if (readPaths(file, tile, zoom, p)) {
			copyPaths(rect, p, list);
			_pathCacheLock.lock();
			_pathCache.insert(key, p);
			_pathCacheLock.unlock();
		} else
			delete p;
	} else {
		copyPaths(rect, cached, list);
		_pathCacheLock.unlock();
	}

	tile->lock.unlock();
}

bool MapData::readPaths(QFile &file, const VectorTile *tile, int zoom,
  QList<Path> *list)
{
	const SubFileInfo &info = _subFiles.at(level(zoom));
	SubFile subfile(file, info.offset, info.size);
	int rows = info.max - info.min + 1;
	QVector<unsigned> paths(rows);
	quint32 blocks, unused, val, cnt = 0;
	quint16 bitmap;
	quint8 sb, flags;
	QByteArray name, houseNumber, reference;


	if (!subfile.seek(tile->offset & OFFSET_MASK))
		return false;

	for (int i = 0; i < rows; i++) {
		if (!(subfile.readVUInt32(unused) && subfile.readVUInt32(val)))
			return false;
		cnt += val;
		paths[i] = cnt;
	}

	if (!subfile.readVUInt32(val))
		return false;
	if (!subfile.seek(subfile.pos() + val))
		return false;

	paths.reserve(paths[zoom - info.min]);

	for (unsigned i = 0; i < paths[zoom - info.min]; i++) {
		qint32 lon = 0, lat = 0;
		Path p(subfile.pos());

		if (!(subfile.readVUInt32(unused) && subfile.readUInt16(bitmap)
		  && subfile.readByte(sb)))
			return false;

		p.point.layer = sb >> 4;
		int tags = sb & 0x0F;
		if (!readTags(subfile, tags, _pathTags, p.point.tags))
			return false;

		if (!subfile.readByte(flags))
			return false;
		if (flags & 0x80) {
			if (!subfile.readString(name))
				return false;
			name = name.split('\r').first();
			p.point.tags.append(Tag(ID_NAME, name));
		}
		if (flags & 0x40) {
			if (!subfile.readString(houseNumber))
				return false;
			p.point.tags.append(Tag(ID_HOUSE, houseNumber));
		}
		if (flags & 0x20) {
			if (!subfile.readString(reference))
				return false;
			p.point.tags.append(Tag(ID_REF, reference));
		}
		if (flags & 0x10) {
			if (!(subfile.readVInt32(lat) && subfile.readVInt32(lon)))
				return false;
		}
		if (flags & 0x08) {
			if (!subfile.readVUInt32(blocks) || !blocks)
				return false;
		} else
			blocks = 1;

		for (unsigned j = 0; j < blocks; j++) {
			if (!readPolygonPath(subfile, tile->pos, flags & 0x04, p.poly))
				return false;
		}
		const QVector<Coordinates> &outline = p.poly.first();
		p.closed = isClosed(outline);
		if (flags & 0x10)
			p.point.coordinates = Coordinates(outline.first().lon() + MD(lon),
			  outline.first().lat() + MD(lat));
		else if (p.closed)
			p.point.coordinates = p.poly.boundingRect().center();

		list->append(p);
	}

	return true;
}

bool MapData::readPoints(QFile &file, const VectorTile *tile, int zoom,
  QList<Point> *list)
{
	const SubFileInfo &info = _subFiles.at(level(zoom));
	SubFile subfile(file, info.offset, info.size);
	int rows = info.max - info.min + 1;
	QVector<unsigned> points(rows);
	quint32 val, unused, cnt = 0;
	quint8 sb, flags;
	QByteArray name, houseNumber;


	if (!subfile.seek(tile->offset & OFFSET_MASK))
		return false;

	for (int i = 0; i < rows; i++) {
		if (!(subfile.readVUInt32(val) && subfile.readVUInt32(unused)))
			return false;
		cnt += val;
		points[i] = cnt;
	}

	if (!subfile.readVUInt32(unused))
		return false;

	list->reserve(points[zoom - info.min]);

	for (unsigned i = 0; i < points[zoom - info.min]; i++) {
		qint32 lat, lon;
		Point p(subfile.pos());

		if (!(subfile.readVInt32(lat) && subfile.readVInt32(lon)))
			return false;
		p.coordinates = Coordinates(tile->pos.lon() + MD(lon),
		  tile->pos.lat() + MD(lat));

		if (!subfile.readByte(sb))
			return false;
		p.layer = sb >> 4;
		int tags = sb & 0x0F;
		if (!readTags(subfile, tags, _pointTags, p.tags))
			return false;

		if (!subfile.readByte(flags))
			return false;
		if (flags & 0x80) {
			if (!subfile.readString(name))
				return false;
			name = name.split('\r').first();
			p.tags.append(Tag(ID_NAME, name));
		}
		if (flags & 0x40) {
			if (!subfile.readString(houseNumber))
				return false;
			p.tags.append(Tag(ID_HOUSE, houseNumber));
		}
		if (flags & 0x20) {
			qint32 elevation;
			if (!subfile.readVInt32(elevation))
				return false;
			p.tags.append(Tag(ID_ELE, QByteArray::number(elevation)));
		}

		list->append(p);
	}

	return true;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Mapsforge::MapData::Tag &tag)
{
	dbg.nospace() << "Tag(" << tag.key << ", " << tag.value << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const MapData::Path &path)
{
	dbg.nospace() << "Path(" << path.poly.boundingRect() << ", "
	  << path.point.tags << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const MapData::Point &point)
{
	dbg.nospace() << "Point(" << point.coordinates << ", " << point.tags << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
