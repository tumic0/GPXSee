#ifndef DEM_H
#define DEM_H

#include <QString>
#include <QCache>
#include <QByteArray>
#include <QMutex>
#include "common/hash.h"
#include "data/area.h"

class Coordinates;

class DEM
{
public:
	class Tile {
	public:
		Tile(int lon, int lat) : _lon(lon), _lat(lat) {}

		int lon() const {return _lon;}
		int lat() const {return _lat;}

		QString lonStr() const;
		QString latStr() const;
		QString baseName() const;

		bool operator==(const Tile &other) const
		{
			return (_lon == other._lon && _lat == other._lat);
		}

	private:
		int _lon, _lat;
	};

	static void setCacheSize(int size);
	static void setDir(const QString &path);
	static void clearCache();

	static double elevation(const Coordinates &c);
	static void lock() {_lock.lock();}
	static void unlock() {_lock.unlock();}

	static QList<Area> tiles();

private:
	typedef QCache<DEM::Tile, QByteArray> TileCache;

	static QByteArray *loadTile(const Tile &tile);

	static QString _dir;
	static TileCache _data;
	static QMutex _lock;
};

inline HASH_T qHash(const DEM::Tile &tile)
{
	return (qHash(tile.lon()) ^ qHash(tile.lat()));
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const DEM::Tile &tile);
#endif // QT_NO_DEBUG

#endif // DEM_H
