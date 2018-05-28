#include <QFileInfo>
#include <QDir>
#include "atlas.h"
#include "offlinemap.h"
#include "onlinemap.h"
#include "jnxmap.h"
#include "mapsource.h"
#include "maplist.h"


bool MapList::loadMap(Map* map, const QString &path, bool dir)
{
	if (map->isValid()) {
		_maps.append(map);
		return true;
	} else {
		if (dir)
			_errorString += path + ": " + map->errorString() + "\n";
		else
			_errorString = map->errorString();

		delete map;
		return false;
	}
}

bool MapList::loadSource(const QString &path, bool dir)
{
	QString err;
	Map *map = MapSource::loadMap(path, err);

	if (!map) {
		if (dir)
			_errorString += path + ": " + err + "\n";
		else
			_errorString = err;
		return false;
	}
	map->setParent(this);

	return loadMap(map, path, dir);
}

bool MapList::loadFile(const QString &path, bool *atlas, bool dir)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();

	if (Atlas::isAtlas(path)) {
		*atlas = true;
		return loadMap(new Atlas(path, this), path, dir);
	} else if (suffix == "xml")
		return loadSource(path, dir);
	else if (suffix == "jnx")
		return loadMap(new JNXMap(path, this), path, dir);
	else
		return loadMap(new OfflineMap(path, this), path, dir);
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
		bool atlas = false;

		if (fi.isDir() && fi.fileName() != "set") {
			if (!loadDirR(fi.absoluteFilePath()))
				ret = false;
		} else if (filter().contains("*." + suffix)) {
			if (!loadFile(fi.absoluteFilePath(), &atlas, true))
				ret = false;
			if (atlas)
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

void MapList::clear()
{
	for (int i = 0; i < _maps.count(); i++)
		delete _maps.at(i);
	_maps.clear();
}

QString MapList::formats()
{
	return
	  tr("Supported files") + " (*.jnx *.map *.tar *.tba *.tif *.tiff *.xml);;"
	  + tr("Garmin JNX maps") + " (*.jnx);;"
	  + tr("OziExplorer maps") + " (*.map);;"
	  + tr("TrekBuddy maps/atlases") + " (*.tar *.tba);;"
	  + tr("GeoTIFF images") + " (*.tif *.tiff);;"
	  + tr("Online map sources") + " (*.xml)";
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.jnx" << "*.map" << "*.tba" << "*.tar" << "*.xml" << "*.tif"
	  << "*.tiff";
	return filter;
}
