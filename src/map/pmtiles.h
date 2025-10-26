#ifndef PMTILES_H
#define PMTILES_H

#include <QByteArray>
#include "common/coordinates.h"

class QFile;
class QPoint;

namespace PMTiles
{
	struct Header {
		quint64 magic;
		quint64 rootOffset;
		quint64 rootLength;
		quint64 metadataOffset;
		quint64 metadataLength;
		quint64 leafOffset;
		quint64 leafLength;
		quint64 tileOffset;
		quint64 tileLength;
		quint64 addressedTiles;
		quint64 tileEntries;
		quint64 tileContents;
		qint32 minLon;
		qint32 minLat;
		qint32 maxLon;
		qint32 maxLat;
		quint8 c;
		quint8 ic;
		quint8 tc;
		quint8 tt;
		quint8 minZ;
		quint8 maxZ;
	};

	struct Directory {
		quint64 tileId;
		quint64 length;
		quint64 offset;
		quint32 runLength;
	};

	bool readHeader(QFile &file, Header &hdr, QString &err);
	const Directory *findDir(const QVector<Directory> &list,
	  quint64 tileId);
	QVector<Directory> readDir(QFile &file, quint64 offset, quint64 size,
	  quint8 compression);
	QByteArray readData(QFile &file, quint64 offset, quint64 size,
	  quint8 compression);
	quint64 id(unsigned zoom, const QPoint &tile);
	inline Coordinates pos(qint32 lon, qint32 lat)
	  {return Coordinates(lon / 10000000.0, lat / 10000000.0);}
}

#endif // PMTILES_H
