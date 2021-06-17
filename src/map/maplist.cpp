#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include "IMG/gmapdata.h"
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
#include "invalidmap.h"
#include "maplist.h"


Map *MapList::loadFile(const QString &path, const Projection &proj, bool *isDir)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();
	Map *map = 0;

	if (Atlas::isAtlas(path)) {
		if (isDir)
			*isDir = true;
		map = new Atlas(path);
	} else if (suffix == "xml") {
		if (MapSource::isMap(path)) {
			map = MapSource::loadMap(path);
		} else if (IMG::GMAPData::isGMAP(path)) {
			map = new IMGMap(path);
			if (isDir)
				*isDir = true;
		}
	} else if (suffix == "jnx")
		map = new JNXMap(path, proj);
	else if (suffix == "tif" || suffix == "tiff")
		map = new GeoTIFFMap(path);
	else if (suffix == "mbtiles")
		map = new MBTilesMap(path);
	else if (suffix == "rmap" || suffix == "rtmap")
		map = new RMap(path);
	else if (suffix == "img")
		map = new IMGMap(path);
	else if (suffix == "map") {
		if (Mapsforge::MapData::isMapsforge(path))
			map = new MapsforgeMap(path);
		else
			map = new OziMap(path);
	} else if (suffix == "tar")
		map = new OziMap(path);
	else if (suffix == "kap")
		map = new BSBMap(path);
	else if (suffix == "kmz")
		map = new KMZMap(path, proj);
	else if (suffix == "aqm")
		map = new AQMMap(path);
	else if (suffix == "sqlitedb")
		map = new SqliteMap(path);
	else if (suffix == "wld" || suffix == "jgw" || suffix == "gfw"
	  || suffix == "pgw" || suffix == "tfw")
		map = new WorldFileMap(path, proj);

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
	  + qApp->translate("MapList", "TwoNav maps") + " (*.rmap *.rtmap);;"
	  + qApp->translate("MapList", "Locus/OsmAnd/RMaps SQLite maps")
		+ " (*.sqlitedb);;"
	  + qApp->translate("MapList", "TrekBuddy maps/atlases") + " (*.tar *.tba);;"
	  + qApp->translate("MapList", "GeoTIFF images") + " (*.tif *.tiff);;"
	  + qApp->translate("MapList", "World-file georeferenced images")
	    + " (*.wld *.jgw *.gfw *.pgw *.tfw);;"
	  + qApp->translate("MapList", "Online map sources") + " (*.xml)";
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.aqm" << "*.gfw" << "*.gmap" << "*.gmapi" << "*.img" << "*.jgw"
	  << "*.jnx" << "*.kap" << "*.kmz" << "*.map" << "*.mbtiles" << "*.pgw"
	  << "*.rmap" << "*.rtmap" << "*.sqlitedb" << "*.tar" << "*.tba" << "*.tfw"
	  << "*.tif" << "*.tiff" << "*.wld" << "*.xml";
	return filter;
}
