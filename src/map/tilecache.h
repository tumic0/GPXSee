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
		Key(const void *object, int zoom, const QPoint &xy)
		  : object(object), zoom(zoom), xy(xy) {}

		bool operator==(const Key &other) const
		{
			return (object == other.object && zoom == other.zoom
			  && xy == other.xy);
		}

	private:
		const void *object;
		int zoom;
		QPoint xy;

		friend HASH_T qHash(const Key &key, HASH_T seed);
	};

	static QPixmap *object(const Key &key) {return _cache.object(key);}
	static bool insert(const Key &key, QPixmap *pixmap);
	static void setCacheLimit(int n) {_cache.setMaxCost(n);}
	static void clear() {return _cache.clear();}

private:
	static QCache<Key, QPixmap> _cache;
};

inline HASH_T qHash(const TileCache::Key &key, HASH_T seed)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QtPrivate::QHashCombine hash;

	seed = hash(seed, key.object);
	seed = hash(seed, key.zoom);
	seed = hash(seed, key.xy);

	return seed;
#else // QT6
	return qHashMulti(seed, key.object, key.zoom, key.xy);
#endif // QT6
}

#endif // TILECACHE_H
