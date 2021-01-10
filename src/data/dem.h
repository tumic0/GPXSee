#ifndef DEM_H
#define DEM_H

#include <QString>
#include <QCache>
#include <QByteArray>
#include "common/config.h"

class QString;
class Coordinates;

class DEM
{
private:
	class Key {
	public:
		Key(int lon, int lat) : _lon(lon), _lat(lat) {}

		int lon() const {return _lon;}
		int lat() const {return _lat;}

		bool operator==(const Key &other) const
		{
			return (_lon == other._lon && _lat == other._lat);
		}

	private:
		int _lon, _lat;
	};

	static QString baseName(const Key &key);
	static QString fileName(const QString &baseName);

	static QString _dir;
	static QCache<Key, QByteArray> _data;

public:
	static void setDir(const QString &path);
	static qreal elevation(const Coordinates &c);

	friend HASH_T qHash(const Key &key);
};

inline HASH_T qHash(const DEM::Key &key)
{
	return (qHash(key.lon()) ^ qHash(key.lat()));
}

#endif // DEM_H
