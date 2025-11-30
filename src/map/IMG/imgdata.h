#ifndef IMG_IMGDATA_H
#define IMG_IMGDATA_H

#include "mapdata_img.h"

class QFile;

namespace IMG {

class IMGData : public MapData
{
public:
	IMGData(const QString &fileName, PolyCache &polyCache,
	  PointCache &pointCache, ElevationCache &demCache, QMutex &lock,
	  QMutex &demLock);

	unsigned blockBits() const {return _blockBits;}
	bool readBlock(QFile *file, int blockNum, char *data) const;

private:
	typedef QMap<QByteArray, VectorTile*> TileMap;

	qint64 read(QFile *file, char *data, qint64 maxSize) const;
	template<class T> bool readValue(QFile *file, T &val) const;
	bool readSubFileBlocks(QFile *file, quint64 offset, SubFile *subFile);
	bool readFAT(QFile *file, TileMap &tileMap);
	bool readIMGHeader(QFile *file);
	bool createTileTree(QFile *file, const TileMap &tileMap);

	quint8 _key;
	unsigned _blockBits;
};

}

#endif // IMG_IMGDATA_H
