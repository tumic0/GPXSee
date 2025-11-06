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
#include "osmdroidmap.h"
#include "gemfmap.h"
#include "oruxmap.h"
#include "encmap.h"
#include "encatlas.h"
#include "coros4map.h"
#include "coros5map.h"
#include "pmtilesmap.h"
#include "invalidmap.h"
#include "maplist.h"


MapList::ParserMap MapList::parsers()
{
	MapList::ParserMap map;

	map.insert("tar", Parser("TrekBuddy TAR atlas", &Atlas::createTAR));
	map.insert("tar", Parser("TrekBuddy TAR map", &OziMap::createTAR));
	map.insert("tba", Parser("TrekBuddy atlas", &Atlas::createTBA));
	map.insert("xml", Parser("Online map", &MapSource::create));
	map.insert("xml", Parser("GARMIN GMAP map", &IMGMap::createGMAP));
	map.insert("img", Parser("GARMIN IMG map", &IMGMap::createIMG));
	map.insert("csm", Parser("COROS 4 map tile", &IMGMap::createIMG));
	map.insert("jnx", Parser("GARMIN JNX map", &JNXMap::create));
	map.insert("tif", Parser("GeoTIFF image", &GeoTIFFMap::create));
	map.insert("tiff", Parser("GeoTIFF image", &GeoTIFFMap::create));
	map.insert("mbtiles", Parser("MBTILES map", &MBTilesMap::create));
	map.insert("rmap", Parser("TwoNav RMAP map", &RMap::create));
	map.insert("rtmap", Parser("TwoNav RMAP map", &RMap::create));
	map.insert("map", Parser("Mapsforge map", &MapsforgeMap::create));
	map.insert("map", Parser("OziExplorer/TrekBuddy map", &OziMap::createMAP));
	map.insert("gmi", Parser("TrekBuddy map", &OziMap::createGMI));
	map.insert("kap", Parser("BSB nautical chart", &BSBMap::create));
	map.insert("kmz", Parser("KML map", &KMZMap::create));
	map.insert("aqm", Parser("AlpineQuest map", &AQMMap::create));
	map.insert("sqlitedb", Parser("RMaps SQLite map", &SqliteMap::create));
	map.insert("wld", Parser("World-file + image", &WorldFileMap::create));
	map.insert("jgw", Parser("World-file + JPG", &WorldFileMap::create));
	map.insert("gfw", Parser("World-file + GIF", &WorldFileMap::create));
	map.insert("pgw", Parser("World-file + PNG", &WorldFileMap::create));
	map.insert("tfw", Parser("World-file + TIFF", &WorldFileMap::create));
	map.insert("qct", Parser("QCT map", &QCTMap::create));
	map.insert("sqlite", Parser("Osmdroid SQLite map", &OsmdroidMap::create));
	map.insert("gemf", Parser("GEMF map", &GEMFMap::create));
	map.insert("otrk2.xml", Parser("Orux map", &OruxMap::create));
	map.insert("000", Parser("ENC chart", &ENCMap::create));
	map.insert("031", Parser("ENC atlas", &ENCAtlas::create));
	map.insert("csa", Parser("COROS 4 map", &Coros4Map::create));
	map.insert("pma", Parser("COROS 5 map", &Coros5Map::create));
	map.insert("pmtiles", Parser("PMTiles map", &PMTilesMap::create));
	map.insert("t", Parser("COROS 5 map tile", &PMTilesMap::create));

	return map;
}

MapList::ParserMap MapList::_parsers = MapList::parsers();

Map *MapList::loadFile(const QString &path, const Projection &proj, bool *isDir)
{
	ParserMap::iterator it;
	QFileInfo fi(Util::displayName(path));
	QString suffix(fi.completeSuffix().toLower());
	QList<QPair<const char*, QString> > errors;

	if ((it = _parsers.find(suffix)) != _parsers.end()) {
		while (it != _parsers.end() && it.key() == suffix) {
			const Parser &p = it.value();

			Map *map = p.cb(path, proj, isDir);
			if (map->isValid())
				return map;
			else {
				errors.append(QPair<const char*, QString>(p.name,
				  map->errorString()));
				delete map;
			}
			++it;
		}

		QString errorString;
		if (errors.size() == 1)
			errorString = errors.first().second;
		else {
			errorString.append("\n");
			for (int i = 0; i < errors.size(); i++)
				errorString.append("\n" + QString(errors.at(i).first) + ": "
				  + errors.at(i).second);
		}

		return new InvalidMap(path, errorString);
	} else {
		for (it = _parsers.begin(); it != _parsers.end(); it++) {
			const Parser &p = it.value();

			Map *map = p.cb(path, proj, isDir);
			if (map->isValid())
				return map;
			else {
				errors.append(QPair<const char*, QString>(p.name,
				  map->errorString()));
				delete map;
			}
		}

		qWarning("%s:", qUtf8Printable(path));
		for (int i = 0; i < errors.size(); i++)
			qWarning("  %s: %s", errors.at(i).first,
			  qUtf8Printable(errors.at(i).second));

		return new InvalidMap(path, "Unknown file format");
	}
}

TreeNode<Map*> MapList::loadDir(const QString &path, const Projection &proj,
  TreeNode<Map*> *parent)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();
#ifdef Q_OS_ANDROID
	TreeNode<Map*> tree(Util::displayName(path));
#else // Q_OS_ANDROID
	TreeNode<Map*> tree(md.dirName());
#endif // Q_OS_ANDROID

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
	  + qApp->translate("MapList", "Electronic Navigational Charts")
		+ " (*.000 *.031);;"
	  + qApp->translate("MapList", "AlpineQuest maps") + " (*.aqm);;"
	  + qApp->translate("MapList", "COROS maps") + " (*.csa *.pma);;"
	  + qApp->translate("MapList", "GEMF maps") + " (*.gemf);;"
	  + qApp->translate("MapList", "Garmin IMG maps")
		+ " (*.csm *.gmap *.gmapi *.img *.xml);;"
	  + qApp->translate("MapList", "Garmin JNX maps") + " (*.jnx);;"
	  + qApp->translate("MapList", "BSB nautical charts") + " (*.kap);;"
	  + qApp->translate("MapList", "KMZ maps") + " (*.kmz);;"
	  + qApp->translate("MapList", "Mapsforge maps") + " (*.map);;"
	  + qApp->translate("MapList", "OziExplorer maps") + " (*.map);;"
	  + qApp->translate("MapList", "MBTiles maps") + " (*.mbtiles);;"
	  + qApp->translate("MapList", "Orux maps") + " (*.otrk2.xml);;"
	  + qApp->translate("MapList", "PMTiles maps") + " (*.pmtiles *.t);;"
	  + qApp->translate("MapList", "QuickChart maps") + " (*.qct);;"
	  + qApp->translate("MapList", "TwoNav maps") + " (*.rmap *.rtmap);;"
	  + qApp->translate("MapList", "Osmdroid SQLite maps") + " (*.sqlite);;"
	  + qApp->translate("MapList", "Locus/OsmAnd/RMaps SQLite maps")
		+ " (*.sqlitedb);;"
	  + qApp->translate("MapList", "TrekBuddy maps/atlases")
		+ " (*.tar *.tba *.gmi *.map);;"
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
