#include <QtGlobal>
#include "tilecache.h"

QCache<TileCache::Key, QPixmap> TileCache::_cache;

bool TileCache::insert(const Key &key, QPixmap *pixmap)
{
	qsizetype cost = (static_cast<quint64>(pixmap->width())
	  * static_cast<quint64>(pixmap->height())
	  * (static_cast<quint64>(pixmap->depth()) >> 3)) >> 10;

	return _cache.insert(key, pixmap, qMax(cost, 1ll));
}
