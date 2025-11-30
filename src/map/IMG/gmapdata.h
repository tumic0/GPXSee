#ifndef IMG_GMAP_H
#define IMG_GMAP_H

#include "mapdata_img.h"

class QXmlStreamReader;
class QDir;

namespace IMG {

class GMAPData : public MapData
{
public:
	GMAPData(const QString &fileName, PolyCache &polyCache,
	  PointCache &pointCache, ElevationCache &demCache, QMutex &lock,
	  QMutex &demLock);

private:
	bool readXML(const QString &path, QString &dataDir, QString &typFile);
	void mapProduct(QXmlStreamReader &reader, QString &dataDir,
	  QString &typFile);
	void subProduct(QXmlStreamReader &reader, QString &dataDir);
	bool loadTile(const QDir &dir);
};

}

#endif // IMG_GMAP_H
