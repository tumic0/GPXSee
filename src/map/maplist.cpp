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
#include "bsbmap.h"
#include "kmzmap.h"
#include "aqmmap.h"
#include "sqlitemap.h"
#include "mapsforgemap.h"
#include "worldfilemap.h"
#include "qctmap.h"
#include "invalidmap.h"
#include "maplist.h"


MapList::ParserMap MapList::parsers()
{
	MapList::ParserMap map;

	map.insert("tar", &Atlas::createTAR);
	map.insert("tar", &OziMap::createTAR);
	map.insert("tba", &Atlas::createTBA);
	map.insert("xml", &MapSource::create);
	map.insert("xml", &IMGMap::createGMAP);
	map.insert("img", &IMGMap::createIMG);
	map.insert("jnx", &JNXMap::create);
	map.insert("tif", &GeoTIFFMap::create);
	map.insert("tiff", &GeoTIFFMap::create);
	map.insert("mbtiles", &MBTilesMap::create);
	map.insert("rmap", &RMap::create);
	map.insert("rtmap", &RMap::create);
	map.insert("map", &MapsforgeMap::create);
	map.insert("map", &OziMap::createMAP);
	map.insert("kap", &BSBMap::create);
	map.insert("kmz", &KMZMap::create);
	map.insert("aqm", &AQMMap::create);
	map.insert("sqlitedb", &SqliteMap::create);
	map.insert("wld", &WorldFileMap::create);
	map.insert("jgw", &WorldFileMap::create);
	map.insert("gfw", &WorldFileMap::create);
	map.insert("pgw", &WorldFileMap::create);
	map.insert("tfw", &WorldFileMap::create);
	map.insert("qct", &QCTMap::create);

	return map;
}

MapList::ParserMap MapList::_parsers = parsers();

Map *MapList::loadFile(const QString &path, const Projection &proj, bool *isDir)
{
	ParserMap::iterator it;
	QFileInfo fi(path);
	QString suffix(fi.suffix().toLower());
	Map *map = 0;

	if ((it = _parsers.find(suffix)) != _parsers.end()) {
		while (it != _parsers.end() && it.key() == suffix) {
			delete map;
			map = it.value()(path, proj, isDir);
			if (map->isValid())
				break;

			++it;
		}
	} else {
		QStringList errors;

		for (it = _parsers.begin(); it != _parsers.end(); it++) {
			map = it.value()(path, proj, isDir);
			if (map->isValid())
				break;
			else {
				errors.append(it.key() + ": " + map->errorString());
				delete map;
				map = 0;
			}
		}

		if (!map) {
			qWarning("Error loading map file: %s:", qPrintable(path));
			for (int i = 0; i < errors.size(); i++)
				qWarning("%s", qPrintable(errors.at(i)));
		}
	}

	return map ? map : new InvalidMap(path, "Unknown file format");
}

TreeNode<Map *> MapList::loadDir(const QString &path, const Projection &proj,
  TreeNode<Map *> *parent)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();
	TreeNode<Map*> tree(md.dirName());

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		QString suffix = fi.suffix().toLower();

		if (fi.isDir()) {
			TreeNode<Map*> child(loadDir(fi.absoluteFilePath(), proj, &tree));
			if (!child.isEmpty())
				tree.addChild(child);
		} else if (filter().contains("*." + suffix)) {
			bool isDir = false;
			Map *map = loadFile(fi.absoluteFilePath(), proj, &isDir);
			if (isDir) {
				if (parent)
					parent->addItem(map);
				else
					tree.addItem(map);
				break;
			} else
				tree.addItem(map);
		}
	}

	return tree;
}

TreeNode<Map *> MapList::loadMaps(const QString &path, const Projection &proj)
{
	if (QFileInfo(path).isDir())
		return loadDir(path, proj);
	else {
		TreeNode<Map*> tree;
		tree.addItem(loadFile(path, proj));
		return tree;
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
	  + qApp->translate("MapList", "Mapsforge maps") + " (*.map);;"
	  + qApp->translate("MapList", "OziExplorer maps") + " (*.map);;"
	  + qApp->translate("MapList", "MBTiles maps") + " (*.mbtiles);;"
	  + qApp->translate("MapList", "QuickChart maps") + " (*.qct);;"
	  + qApp->translate("MapList", "TwoNav maps") + " (*.rmap *.rtmap);;"
	  + qApp->translate("MapList", "Locus/OsmAnd/RMaps SQLite maps")
		+ " (*.sqlitedb);;"
	  + qApp->translate("MapList", "TrekBuddy maps/atlases") + " (*.tar *.tba);;"
	  + qApp->translate("MapList", "GeoTIFF images") + " (*.tif *.tiff);;"
	  + qApp->translate("MapList", "World-file georeferenced images")
	    + " (*.wld *.jgw *.gfw *.pgw *.tfw);;"
	  + qApp->translate("MapList", "Online map sources") + " (*.xml);;"
	  + qApp->translate("MapList", "All files") + " (*)";
}

QStringList MapList::filter()
{
	QStringList filter;
	QString last;

	for (ParserMap::iterator it = _parsers.begin(); it != _parsers.end(); it++) {
		if (it.key() != last)
			filter << "*." + it.key();
		last = it.key();
	}

	return filter;
}
