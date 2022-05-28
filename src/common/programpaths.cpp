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
#define SYMBOLS_DIR      "symbols"

#define ELLIPSOID_FILE   "ellipsoids.csv"
#define GCS_FILE         "gcs.csv"
#define PCS_FILE         "pcs.csv"
#define TYP_FILE         "style.typ"
#define RENDERTHEME_FILE "style.xml"

#ifdef Q_OS_ANDROID
#define DATA_LOCATION QStandardPaths::GenericDataLocation
#else // Q_OS_ANDROID
#define DATA_LOCATION QStandardPaths::AppDataLocation
#endif // Q_OS_ANDROID

#ifdef Q_OS_ANDROID
static QString assetsPath(const QString &path, const QString &dir)
{
	QDir pd(path);

	if (pd.isAbsolute() && pd.exists())
		return pd.absolutePath();
	else
		return QString("assets://") + dir;
}
#endif // Q_OS_ANDROID

QString ProgramPaths::mapDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(MAP_DIR);
	else
#ifdef Q_OS_ANDROID
		return assetsPath(QStandardPaths::locate(DATA_LOCATION, MAP_DIR,
		  QStandardPaths::LocateDirectory), MAP_DIR);
#else // Q_OS_ANDROID
		return QStandardPaths::locate(DATA_LOCATION,
		  MAP_DIR, QStandardPaths::LocateDirectory);
#endif // Q_OS_ANDROID
}

QString ProgramPaths::poiDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(POI_DIR);
	else
		return QStandardPaths::locate(DATA_LOCATION, POI_DIR,
		  QStandardPaths::LocateDirectory);
}

QString ProgramPaths::csvDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(CSV_DIR);
	else
#ifdef Q_OS_ANDROID
		return assetsPath(QStandardPaths::locate(DATA_LOCATION, CSV_DIR,
		  QStandardPaths::LocateDirectory), CSV_DIR);
#else // Q_OS_ANDROID
		return QStandardPaths::locate(DATA_LOCATION, CSV_DIR,
		  QStandardPaths::LocateDirectory);
#endif // Q_OS_ANDROID
}

QString ProgramPaths::demDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(DEM_DIR);
	else
		return QStandardPaths::locate(DATA_LOCATION, DEM_DIR,
		  QStandardPaths::LocateDirectory);
}

QString ProgramPaths::styleDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(STYLE_DIR);
	else
		return QStandardPaths::locate(DATA_LOCATION, STYLE_DIR,
		  QStandardPaths::LocateDirectory);
}

QString ProgramPaths::symbolsDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(SYMBOLS_DIR);
	else
#ifdef Q_OS_ANDROID
		return assetsPath(QStandardPaths::locate(DATA_LOCATION, SYMBOLS_DIR,
		  QStandardPaths::LocateDirectory), SYMBOLS_DIR);
#else // Q_OS_ANDROID
		return QStandardPaths::locate(DATA_LOCATION, SYMBOLS_DIR,
		  QStandardPaths::LocateDirectory);
#endif // Q_OS_ANDROID
}

QString ProgramPaths::tilesDir()
{
	return QDir(QStandardPaths::writableLocation(
	  QStandardPaths::CacheLocation)).filePath(TILES_DIR);
}

QString ProgramPaths::translationsDir()
{
	return QStandardPaths::locate(DATA_LOCATION, TRANSLATIONS_DIR,
	  QStandardPaths::LocateDirectory);
}

QString ProgramPaths::ellipsoidsFile()
{
	return QDir(csvDir()).filePath(ELLIPSOID_FILE);
}

QString ProgramPaths::gcsFile()
{
	return QDir(csvDir()).filePath(GCS_FILE);
}

QString ProgramPaths::pcsFile()
{
	return QDir(csvDir()).filePath(PCS_FILE);
}

QString ProgramPaths::typFile()
{
	return QDir(styleDir()).filePath(TYP_FILE);
}

QString ProgramPaths::renderthemeFile()
{
	return QDir(styleDir()).filePath(RENDERTHEME_FILE);
}
