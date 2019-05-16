#include <QtGlobal>
#include <QDir>
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


#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)

#include <QApplication>

#if defined(Q_OS_WIN32)
#define USER_DIR        QDir::homePath() + QString("/AppData/Roaming/") \
						  + qApp->applicationName()
#define GLOBAL_DIR      QApplication::applicationDirPath()
#elif defined(Q_OS_MAC)
#define USER_DIR        QDir::homePath() \
						  + QString("/Library/Application Support/") \
						  + qApp->applicationName()
#define GLOBAL_DIR      QApplication::applicationDirPath() \
						  + QString("/../Resources")
#else
#define USER_DIR        QDir::homePath() + QString("/.local/share/") \
						  + qApp->applicationName()
#define GLOBAL_DIR      QString(PREFIX "/share/") + qApp->applicationName()
#endif

static QString dir(const QString &dirName, bool writable = false)
{
	QDir userDir(QDir(USER_DIR).filePath(dirName));

	if (writable || userDir.exists())
		return userDir.absolutePath();
	else {
		QDir globalDir(QDir(GLOBAL_DIR).filePath(dirName));

		if (globalDir.exists())
			return globalDir.absolutePath();
		else
			return QString();
	}
}

static QString file(const QString &path, const QString &fileName)
{
	if (path.isNull())
		return QString();

	QFileInfo fi(QDir(path).filePath(fileName));
	return fi.exists() ? fi.absoluteFilePath() : QString();
}

QString ProgramPaths::mapDir(bool writable)
{
	return dir(MAP_DIR, writable);
}

QString ProgramPaths::poiDir(bool writable)
{
	return dir(POI_DIR, writable);
}

QString ProgramPaths::csvDir(bool writable)
{
	return dir(CSV_DIR, writable);
}

QString ProgramPaths::demDir(bool writable)
{
	return dir(DEM_DIR, writable);
}

QString ProgramPaths::styleDir(bool writable)
{
	return dir(STYLE_DIR, writable);
}

QString ProgramPaths::tilesDir()
{
#if defined(Q_OS_WIN32)
	return QDir::homePath() + QString("/AppData/Local/")
	  + qApp->applicationName() + QString("/cache/") + QString(TILES_DIR);
#elif defined(Q_OS_MAC)
	return QDir::homePath() + QString("/Library/Caches/")
	  + qApp->applicationName() + QString("/" TILES_DIR);
#else
	return QDir::homePath() + QString("/.cache/") + qApp->applicationName()
	  + QString("/" TILES_DIR);
#endif
}

QString ProgramPaths::translationsDir()
{
	return dir(TRANSLATIONS_DIR);
}

QString ProgramPaths::ellipsoidsFile()
{
	return file(dir(CSV_DIR),  ELLIPSOID_FILE);
}

QString ProgramPaths::gcsFile()
{
	return file(dir(CSV_DIR), GCS_FILE);
}

QString ProgramPaths::pcsFile()
{
	return file(dir(CSV_DIR), PCS_FILE);
}

QString ProgramPaths::typFile()
{
	return file(dir(STYLE_DIR), TYP_FILE);
}

#else // QT_VERSION < 5

#include <QStandardPaths>

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

#endif // QT_VERSION < 5
