#include <QtGlobal>
#include <QDir>
#include <QStandardPaths>
#include "programpaths.h"


#define MAP_DIR          "maps"
#define POI_DIR          "POI"
#define CSV_DIR          "csv"
#define DEM_DIR          "DEM"
#define TILES_DIR        "tiles"
#define TRANSLATIONS_DIR "translations"
#define STYLE_DIR        "style"
#define ELLIPSOID_FILE   "ellipsoids.csv"
#define GCS_FILE         "gcs.csv"
#define PCS_FILE         "pcs.csv"
#define TYP_FILE         "style.typ"
#define RENDERTHEME_FILE "style.xml"


QString ProgramPaths::mapDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(MAP_DIR);
	else
		return QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  MAP_DIR, QStandardPaths::LocateDirectory);
}

QString ProgramPaths::poiDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(POI_DIR);
	else
		return QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  POI_DIR, QStandardPaths::LocateDirectory);
}

QString ProgramPaths::csvDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(CSV_DIR);
	else
		return QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  CSV_DIR, QStandardPaths::LocateDirectory);
}

QString ProgramPaths::demDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(DEM_DIR);
	else
		return QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  DEM_DIR, QStandardPaths::LocateDirectory);
}

QString ProgramPaths::styleDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(STYLE_DIR);
	else
		return QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  STYLE_DIR, QStandardPaths::LocateDirectory);
}

QString ProgramPaths::tilesDir()
{
	return QDir(QStandardPaths::writableLocation(
	  QStandardPaths::CacheLocation)).filePath(TILES_DIR);
}

QString ProgramPaths::translationsDir()
{
	return QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  TRANSLATIONS_DIR, QStandardPaths::LocateDirectory);
}

QString ProgramPaths::ellipsoidsFile()
{
	return QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  CSV_DIR "/" ELLIPSOID_FILE, QStandardPaths::LocateFile);
}

QString ProgramPaths::gcsFile()
{
	return QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  CSV_DIR "/" GCS_FILE, QStandardPaths::LocateFile);
}

QString ProgramPaths::pcsFile()
{
	return QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  CSV_DIR "/" PCS_FILE, QStandardPaths::LocateFile);
}

QString ProgramPaths::typFile()
{
	return QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  STYLE_DIR "/" TYP_FILE, QStandardPaths::LocateFile);
}

QString ProgramPaths::renderthemeFile()
{
	return QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  STYLE_DIR "/" RENDERTHEME_FILE, QStandardPaths::LocateFile);
}
