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
#include "maplist.h"


Map *MapList::loadFile(const QString &path, QString &errorString,
  bool *terminate)
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
			if (!(map = MapSource::loadMap(path, errorString)))
				return 0;
		} else if (GMAP::isGMAP(path)) {
			if (terminate)
				*terminate = true;
			map = new IMGMap(path);
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

	if (map && map->isValid())
		return map;
	else {
		errorString = (map) ? map->errorString() : "Unknown file format";
		delete map;
		return 0;
	}
}

QList<Map*> MapList::loadDir(const QString &path, QString &errorString)
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
			list.append(loadDir(fi.absoluteFilePath(), errorString));
		else if (filter().contains("*." + suffix)) {
			Map *map = loadFile(fi.absoluteFilePath(), errorString, &terminate);
			if (map)
				list.append(map);
			else
				qWarning("%s: %s", qPrintable(fi.absoluteFilePath()),
				  qPrintable(errorString));
			if (terminate)
				break;
		}
	}

	return list;
}

QList<Map*> MapList::loadMaps(const QString &path, QString &errorString)
{
	if (QFileInfo(path).isDir())
		return loadDir(path, errorString);
	else {
		QList<Map*> list;
		Map *map = loadFile(path, errorString, 0);
		if (map)
			list.append(map);
		return list;
	}
}

QString MapList::formats()
{
	return
	  qApp->translate("MapList", "Supported files")
		+ " (" + filter().join(" ") + ");;"
	  + qApp->translate("MapList", "Garmin IMG maps")
		+ " (*.gmap *.gmapi *.img *.xml);;"
	  + qApp->translate("MapList", "Garmin JNX maps") + " (*.jnx);;"
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
	filter << "*.gmap" << "*.gmapi" << "*.img" << "*.jnx" << "*.map"
	  << "*.mbtiles" << "*.rmap" << "*.rtmap" << "*.tar" << "*.tba" << "*.tif"
	  << "*.tiff"  << "*.xml";
	return filter;
}
