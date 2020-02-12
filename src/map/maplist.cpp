#include <QFileInfo>
#include <QDir>
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


bool MapList::loadMap(Map *map, const QString &path, bool dir)
{
	if (map && map->isValid()) {
		_maps.append(map);
		return true;
	} else if (map) {
		if (dir)
			_errorString += path + ": " + map->errorString() + "\n";
		else
			_errorString = map->errorString();
		return false;
	} else {
		if (dir)
			_errorString += path + ": " + "Unknown map format\n";
		else
			_errorString = "Unknown map format";
		return false;
	}
}

Map *MapList::loadSource(const QString &path, bool dir)
{
	QString err;
	Map *map = MapSource::loadMap(path, err);

	if (!map) {
		if (dir)
			_errorString += path + ": " + err + "\n";
		else
			_errorString = err;
	} else
		map->setParent(this);

	return map;
}

bool MapList::loadFile(const QString &path, bool *terminate, bool dir)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();
	Map *map = 0;

	if (Atlas::isAtlas(path)) {
		*terminate = true;
		map = new Atlas(path, this);
	} else if (suffix == "xml") {
		if (MapSource::isMap(path) && !(map = loadSource(path, dir)))
			return false;
		else if (GMAP::isGMAP(path)) {
			*terminate = true;
			map = new IMGMap(path);
		}
	} else if (suffix == "jnx")
		map = new JNXMap(path, this);
	else if (suffix == "tif" || suffix == "tiff")
		map = new GeoTIFFMap(path, this);
	else if (suffix == "mbtiles")
		map = new MBTilesMap(path, this);
	else if (suffix == "rmap" || suffix == "rtmap")
		map = new RMap(path, this);
	else if (suffix == "img")
		map = new IMGMap(path, this);
	else
		map = new OziMap(path, this);

	if (!loadMap(map, path, dir)) {
		delete map;
		return false;
	}

	return true;
}

bool MapList::loadDirR(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();
	bool ret = true;

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		QString suffix = fi.suffix().toLower();
		bool terminate = false;

		if (fi.isDir() && fi.fileName() != "set") {
			if (!loadDirR(fi.absoluteFilePath()))
				ret = false;
		} else if (filter().contains("*." + suffix)) {
			if (!loadFile(fi.absoluteFilePath(), &terminate, true))
				ret = false;
			if (terminate)
				break;
		}
	}

	return ret;
}

bool MapList::loadFile(const QString &path)
{
	bool atlas;

	_errorString.clear();
	return loadFile(path, &atlas, false);
}

bool MapList::loadDir(const QString &path)
{
	_errorString.clear();
	return loadDirR(path);
}

QString MapList::formats()
{
	return
	  tr("Supported files")
	  + " (*.gmap *.gmapi *.img *.jnx *.map *.mbtiles *.rmap *.rtmap *.tar *.tba *.tif *.tiff *.xml);;"
	  + tr("Garmin IMG maps") + " (*.gmap *.gmapi *.img *.xml);;"
	  + tr("Garmin JNX maps") + " (*.jnx);;"
	  + tr("OziExplorer maps") + " (*.map);;"
	  + tr("MBTiles maps") + " (*.mbtiles);;"
	  + tr("TrekBuddy maps/atlases") + " (*.tar *.tba);;"
	  + tr("GeoTIFF images") + " (*.tif *.tiff);;"
	  + tr("TwoNav maps") + " (*.rmap *.rtmap);;"
	  + tr("Online map sources") + " (*.xml)";
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.img" << "*.jnx" << "*.map" << "*.mbtiles" << "*.rmap"
	  << "*.rtmap" << "*.tar" << "*.tba" << "*.tif" << "*.tiff"  << "*.xml";
	return filter;
}
