#ifndef DEM_H
#define DEM_H

#include <QMap>
#include <QByteArray>
#include <QDir>

class QDir;
class Coordinates;

class DEM
{
public:
	DEM() {}
	DEM(const QDir &dir) : _dir(dir) {}
	qreal elevation(const Coordinates &c);

private:
	struct Key {
		int lon;
		int lat;

		Key(int lon, int lat) : lon(lon), lat(lat) {}

		bool operator<(const Key &other) const
		{
			if (lon < other.lon)
				return true;
			else if (lon > other.lon)
				return false;
			else
				return (lat < other.lat);
		}

	};

	QString fileName(const Key &key) const;

	QDir _dir;
	QMap<Key, QByteArray> _data;
};

#endif // DEM_H
