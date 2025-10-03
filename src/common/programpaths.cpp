#include <QtGlobal>
#include <QDir>
#include <QStandardPaths>
#include "programpaths.h"


#define MAP_DIR          "maps"
#define POI_DIR          "POI"
#define CRS_DIR          "CRS"
#define DEM_DIR          "DEM"
#define TILES_DIR        "tiles"
#define TRANSLATIONS_DIR "translations"
#define STYLE_DIR        "style"
#define SYMBOLS_DIR      "symbols"

#define ELLIPSOIDS_FILE  "ellipsoids.csv"
#define PROJECTIONS_FILE "projections.csv"
#define GCS_FILE         "gcs.csv"
#define PCS_FILE         "pcs.csv"

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

QString ProgramPaths::crsDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(DATA_LOCATION))
		  .filePath(CRS_DIR);
	else
#ifdef Q_OS_ANDROID
		return assetsPath(QStandardPaths::locate(DATA_LOCATION, CRS_DIR,
		  QStandardPaths::LocateDirectory), CRS_DIR);
#else // Q_OS_ANDROID
		return QStandardPaths::locate(DATA_LOCATION, CRS_DIR,
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
#ifdef Q_OS_ANDROID
	return assetsPath(QStandardPaths::locate(DATA_LOCATION, TRANSLATIONS_DIR,
	  QStandardPaths::LocateDirectory), TRANSLATIONS_DIR);
#else // Q_OS_ANDROID
	return QStandardPaths::locate(DATA_LOCATION, TRANSLATIONS_DIR,
	  QStandardPaths::LocateDirectory);
#endif // Q_OS_ANDROID
}

QString ProgramPaths::ellipsoidsFile()
{
	QString dir(crsDir());
	return dir.isEmpty() ? QString() : QDir(dir).filePath(ELLIPSOIDS_FILE);
}

QString ProgramPaths::gcsFile()
{
	QString dir(crsDir());
	return dir.isEmpty() ? QString() : QDir(dir).filePath(GCS_FILE);
}

QString ProgramPaths::projectionsFile()
{
	QString dir(crsDir());
	return dir.isEmpty() ? QString() : QDir(dir).filePath(PROJECTIONS_FILE);
}

QString ProgramPaths::pcsFile()
{
	QString dir(crsDir());
	return dir.isEmpty() ? QString() : QDir(crsDir()).filePath(PCS_FILE);
}
