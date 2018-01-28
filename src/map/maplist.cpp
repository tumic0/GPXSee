#include <QFileInfo>
#include <QDir>
#include "common/range.h"
#include "atlas.h"
#include "offlinemap.h"
#include "onlinemap.h"
#include "omd.h"
#include "maplist.h"


bool MapList::loadList(const QString &path, bool dir)
{
	OMD omd;

	if (!omd.loadFile(path)) {
		if (dir)
			_errorString += path + ": " + omd.errorString() + "\n";
		else
			_errorString = omd.errorString();
		return false;
	}

	_maps += omd.maps();
	for (int i = 0; i < omd.maps().size(); i++)
		omd.maps()[i]->setParent(this);

	return true;
}

bool MapList::loadMap(const QString &path, bool dir)
{
	OfflineMap *map = new OfflineMap(path, this);

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

bool MapList::loadAtlas(const QString &path, bool dir)
{
	Atlas *atlas = new Atlas(path, this);

	if (atlas->isValid()) {
		_maps.append(atlas);
		return true;
	} else {
		if (dir)
			_errorString += path + ": " + atlas->errorString() + "\n";
		else
			_errorString = atlas->errorString();
		delete atlas;
		return false;
	}
}

bool MapList::loadFile(const QString &path, bool *atlas, bool dir)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();

	if (Atlas::isAtlas(path)) {
		*atlas = true;
		return loadAtlas(path, dir);
	} else if (suffix == "xml") {
		*atlas = false;
		return loadList(path, dir);
	} else {
		*atlas = false;
		return loadMap(path, dir);
	}
}

bool MapList::loadFile(const QString &path)
{
	bool atlas;
	return loadFile(path, &atlas, false);
}

bool MapList::loadDir(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();
	bool atlas, ret = true;

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		QString suffix = fi.suffix().toLower();

		if (fi.isDir() && fi.fileName() != "set") {
			if (!loadDir(fi.absoluteFilePath()))
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

void MapList::clear()
{
	for (int i = 0; i < _maps.count(); i++)
		delete _maps.at(i);
	_maps.clear();
}

QString MapList::formats()
{
	return tr("Supported files (*.map *.tar *.tba *.tif *.tiff *.xml)") + ";;"
	  + tr("OziExplorer maps (*.map)") + ";;"
	  + tr("TrekBuddy maps/atlases (*.tar *.tba)") + ";;"
	  + tr("GeoTIFF images (*.tif *.tiff)") + ";;"
	  + tr("Online map definitions (*.xml)");
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.map" << "*.tba" << "*.tar" << "*.xml" << "*.tif" << "*.tiff";
	return filter;
}
