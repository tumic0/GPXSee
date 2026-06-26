#ifndef TILECACHE_H
#define TILECACHE_H

#include <QCache>
#include <QPixmap>
#include "common/hash.h"

class Map;

class TileCache
{
public:
	class Key
	{
	public:
		Key(const void *object, int zoom, const QPoint &tile)
		  : object(object), zoom(zoom),
			tile(static_cast<quint64>(tile.x()) << 32 | tile.y()) {}
		Key(const void *object, int zoom, quint64 tile)
		  : object(object), zoom(zoom), tile(tile) {}

		bool operator==(const Key &other) const
		{
			return (object == other.object && zoom == other.zoom
			  && tile == other.tile);
		}

	private:
		const void *object;
		int zoom;
		quint64 tile;

		friend HASH_T qHash(const Key &key, HASH_T seed);
	};

	static QPixmap *object(const Key &key) {return _cache.object(key);}
	static bool insert(const Key &key, QPixmap *pixmap);
	static void setCacheLimit(int n) {_cache.setMaxCost(n);}
	static void clear() {return _cache.clear();}

private:
	static QCache<Key, QPixmap> _cache;
};

inline HASH_T qHash(const TileCache::Key &key, HASH_T seed = 0)
{
	return qHashMulti(seed, key.object, key.zoom, key.tile);
}

#endif // TILECACHE_H
