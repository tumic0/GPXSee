#ifndef IMG_GMAP_H
#define IMG_GMAP_H

#include "mapdata.h"

class QXmlStreamReader;
class QDir;

namespace IMG {

class GMAPData : public MapData
{
public:
	GMAPData(const QString &fileName);

private:
	bool readXML(const QString &path, QString &dataDir, QString &typFile);
	void mapProduct(QXmlStreamReader &reader, QString &dataDir,
	  QString &typFile);
	void subProduct(QXmlStreamReader &reader, QString &dataDir);
	bool loadTile(const QDir &dir);
};

}

#endif // IMG_GMAP_H
