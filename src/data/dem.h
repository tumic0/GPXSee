#ifndef DEM_H
#define DEM_H

#include <QString>
#include <QCache>
#include <QByteArray>
#include "common/hash.h"
#include "area.h"

class QString;
class Coordinates;
class RectC;

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
	static qreal elevation(const Coordinates &c);

	static QList<Area> tiles();

private:
	typedef QCache<DEM::Tile, QByteArray> TileCache;

	static QString fileName(const QString &baseName);

	static QString _dir;
	static TileCache _data;
};

inline HASH_T qHash(const DEM::Tile &tile)
{
	return (qHash(tile.lon()) ^ qHash(tile.lat()));
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const DEM::Tile &tile);
#endif // QT_NO_DEBUG

#endif // DEM_H
