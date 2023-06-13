#ifndef MAPSFORGE_MAPDATA_H
#define MAPSFORGE_MAPDATA_H

#include <QFile>
#include <QCache>
#include <QMutex>
#include "common/hash.h"
#include "common/rectc.h"
#include "common/rtree.h"
#include "common/range.h"
#include "common/polygon.h"

#define ID_NAME   1
#define ID_HOUSE  2
#define ID_REF    3
#define ID_ELE    4

namespace Mapsforge {

class SubFile;

class MapData
{
public:
	MapData(const QString &path);
	~MapData();

	struct Tag {
		Tag() {}
		Tag(unsigned key, const QByteArray &value) : key(key), value(value) {}

		bool operator==(const Tag &other) const
		  {return (key == other.key && value == other.value);}

		unsigned key;
		QByteArray value;
	};

	struct Point {
		Point(const Coordinates &c) : id(qHash(c)), coordinates(c) {}

		quint64 id;
		Coordinates coordinates;
		QVector<Tag> tags;
		int layer;
	};

	struct Path {
		Polygon poly;
		QVector<Tag> tags;
		Coordinates labelPos;
		int layer;
		bool closed;

		bool operator<(const Path &other) const
		  {return layer < other.layer;}
	};

	RectC bounds() const;
	Range zooms() const
	  {return Range(_subFiles.first().min, _subFiles.last().max);}
	int tileSize() const
	  {return _tileSize < 384 ? _tileSize << 1 : _tileSize;}

	void points(const RectC &rect, int zoom, QList<Point> *list);
	void paths(const RectC &searchRect, const RectC &boundsRect, int zoom,
	  QList<Path> *set);
	unsigned tagId(const QByteArray &name) const {return _keys.value(name);}

	void load();
	void clear();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	struct SubFileInfo {
		quint8 base;
		quint8 min;
		quint8 max;
		quint64 offset;
		quint64 size;
	};

	struct VectorTile {
		VectorTile(size_t offset, const RectC &rect)
		  : offset(offset), pos(rect.topLeft()) {}

		size_t offset;
		Coordinates pos;
	};

	struct PathCTX {
		PathCTX(MapData *data, const RectC &rect, int zoom, QList<Path> *list)
		  : data(data), rect(rect), zoom(zoom), list(list) {}

		MapData *data;
		const RectC &rect;
		int zoom;
		QList<Path> *list;
	};

	struct PointCTX {
		PointCTX(MapData *data, const RectC &rect, int zoom, QList<Point> *list)
		  : data(data), rect(rect), zoom(zoom), list(list) {}

		MapData *data;
		const RectC &rect;
		int zoom;
		QList<Point> *list;
	};

	struct Key {
		Key(const VectorTile *tile, int zoom) : tile(tile), zoom(zoom) {}
		bool operator==(const Key &other) const
		  {return tile == other.tile && zoom == other.zoom;}

		const VectorTile *tile;
		int zoom;
	};

	struct TagSource {
		TagSource() {}
		TagSource(const QByteArray &str)
		{
			QList<QByteArray> l(str.split('='));
			if (l.size() == 2) {
				key = l.at(0);
				value = l.at(1);
			}
		}

		QByteArray key;
		QByteArray value;
		unsigned id;
	};

	typedef RTree<VectorTile *, double, 2> TileTree;

	bool readZoomInfo(SubFile &hdr);
	bool readTagInfo(SubFile &hdr);
	bool readTagInfo(SubFile &hdr, QVector<TagSource> &tags);
	bool readMapInfo(SubFile &hdr, QByteArray &projection, bool &debugMap);
	bool readHeader(QFile &file);
	bool readSubFiles();
	void clearTiles();

	int level(int zoom) const;
	void paths(const VectorTile *tile, const RectC &rect, int zoom,
	  QList<Path> *list);
	void points(const VectorTile *tile, const RectC &rect, int zoom,
	  QList<Point> *list);
	bool readPaths(const VectorTile *tile, int zoom, QList<Path> *list);
	bool readPoints(const VectorTile *tile, int zoom, QList<Point> *list);

	static bool readTags(SubFile &subfile, int count,
	  const QVector<TagSource> &tags, QVector<Tag> &list);
	static bool pathCb(VectorTile *tile, void *context);
	static bool pointCb(VectorTile *tile, void *context);

	friend HASH_T qHash(const MapData::Key &key);

	QFile _pointFile, _pathFile;
	RectC _bounds;
	quint16 _tileSize;
	QVector<SubFileInfo> _subFiles;
	QVector<TagSource> _pointTags, _pathTags;
	QList<TileTree*> _tiles;
	QHash<QByteArray, unsigned> _keys;

	QCache<Key, QList<Path> > _pathCache;
	QCache<Key, QList<Point> > _pointCache;
	QMutex _pathLock, _pointLock;

	bool _valid;
	QString _errorString;
};

inline HASH_T qHash(const MapData::Key &key)
{
	return ::qHash(key.tile) ^ ::qHash(key.zoom);
}

inline HASH_T qHash(const MapData::Tag &tag)
{
	return ::qHash(tag.key) ^ ::qHash(tag.value);
}

}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Mapsforge::MapData::Tag &tag);
QDebug operator<<(QDebug dbg, const Mapsforge::MapData::Path &path);
QDebug operator<<(QDebug dbg, const Mapsforge::MapData::Point &point);
#endif // QT_NO_DEBUG

#endif // MAPSFORGE_MAPDATA_H
