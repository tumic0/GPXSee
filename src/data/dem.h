#ifndef DEM_H
#define DEM_H

#include <QString>
#include <QMap>
#include <QByteArray>

class QString;
class Coordinates;

class DEM
{
public:
	static void setDir(const QString &path);
	static qreal elevation(const Coordinates &c);

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

	static QString fileName(const Key &key);

	static QString _dir;
	static QMap<Key, QByteArray> _data;
};

#endif // DEM_H
