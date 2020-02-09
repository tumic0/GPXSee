#ifndef GMAP_H
#define GMAP_H

#include "mapdata.h"

class QXmlStreamReader;
class QDir;

class GMAP : public MapData
{
public:
	GMAP(const QString &fileName);

	QString fileName() const {return _fileName;}

	static bool isGMAP(const QString &path);

private:
	QString _fileName;

	bool readXML(const QString &path, QString &dataDir, QString &typFile,
	  QString &baseMap);
	void mapProduct(QXmlStreamReader &reader, QString &dataDir,
	  QString &typFile, QString &baseMap);
	void subProduct(QXmlStreamReader &reader, QString &dataDir,
	  QString &baseMap);
	bool loadTile(const QDir &dir, quint16 &id);
};

#endif // GMAP_H
