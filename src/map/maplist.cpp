#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include "atlas.h"
#include "ozimap.h"
#include "jnxmap.h"
#include "geotiffmap.h"
#include "mapsource.h"
#include "mbtilesmap.h"
#include "rmap.h"
#include "imgmap.h"
#include "IMG/gmap.h"
#include "bsbmap.h"
#include "kmzmap.h"
#include "aqmmap.h"
#include "invalidmap.h"
#include "maplist.h"


Map *MapList::loadFile(const QString &path, bool *terminate)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();
	Map *map = 0;

	if (Atlas::isAtlas(path)) {
		if (terminate)
			*terminate = true;
		map = new Atlas(path);
	} else if (suffix == "xml") {
		if (MapSource::isMap(path)) {
			map = MapSource::loadMap(path);
		} else if (GMAP::isGMAP(path)) {
			map = new IMGMap(path);
			if (terminate)
				*terminate = true;
		}
	} else if (suffix == "jnx")
		map = new JNXMap(path);
	else if (suffix == "tif" || suffix == "tiff")
		map = new GeoTIFFMap(path);
	else if (suffix == "mbtiles")
		map = new MBTilesMap(path);
	else if (suffix == "rmap" || suffix == "rtmap")
		map = new RMap(path);
	else if (suffix == "img")
		map = new IMGMap(path);
	else if (suffix == "map" || suffix == "tar")
		map = new OziMap(path);
	else if (suffix == "kap")
		map = new BSBMap(path);
	else if (suffix == "kmz")
		map = new KMZMap(path);
	else if (suffix == "aqm")
		map = new AQMMap(path);

	return map ? map : new InvalidMap(path, "Unknown file format");
}

QList<Map*> MapList::loadDir(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();
	QList<Map*> list;

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		QString suffix = fi.suffix().toLower();
		bool terminate = false;

		if (fi.isDir() && fi.fileName() != "set")
			list.append(loadDir(fi.absoluteFilePath()));
		else if (filter().contains("*." + suffix)) {
			list.append(loadFile(fi.absoluteFilePath(), &terminate));
			if (terminate)
				break;
		}
	}

	return list;
}

QList<Map*> MapList::loadMaps(const QString &path)
{
	if (QFileInfo(path).isDir())
		return loadDir(path);
	else {
		QList<Map*> list;
		list.append(loadFile(path, 0));
		return list;
	}
}

QString MapList::formats()
{
	return
	  qApp->translate("MapList", "Supported files")
		+ " (" + filter().join(" ") + ");;"
	  + qApp->translate("MapList", "AlpineQuest maps") + " (*.aqm);;"
	  + qApp->translate("MapList", "Garmin IMG maps")
		+ " (*.gmap *.gmapi *.img *.xml);;"
	  + qApp->translate("MapList", "Garmin JNX maps") + " (*.jnx);;"
	  + qApp->translate("MapList", "BSB nautical charts") + " (*.kap);;"
	  + qApp->translate("MapList", "KMZ maps") + " (*.kmz);;"
	  + qApp->translate("MapList", "OziExplorer maps") + " (*.map);;"
	  + qApp->translate("MapList", "MBTiles maps") + " (*.mbtiles);;"
	  + qApp->translate("MapList", "TrekBuddy maps/atlases") + " (*.tar *.tba);;"
	  + qApp->translate("MapList", "GeoTIFF images") + " (*.tif *.tiff);;"
	  + qApp->translate("MapList", "TwoNav maps") + " (*.rmap *.rtmap);;"
	  + qApp->translate("MapList", "Online map sources") + " (*.xml)";
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.aqm" << "*.gmap" << "*.gmapi" << "*.img" << "*.jnx" << "*.kap"
	  << "*.kmz" << "*.map" << "*.mbtiles" << "*.rmap" << "*.rtmap" << "*.tar"
	  << "*.tba" << "*.tif" << "*.tiff"  << "*.xml";
	return filter;
}
