#include <QtGlobal>
#include <QDir>
#include "programpaths.h"


#define MAP_DIR          "maps"
#define POI_DIR          "POI"
#define CSV_DIR          "csv"
#define TILES_DIR        "tiles"
#define TRANSLATIONS_DIR "translations"
#define ELLIPSOID_FILE   "ellipsoids.csv"
#define GCS_FILE         "gcs.csv"
#define PCS_FILE         "pcs.csv"


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)

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
#define GLOBAL_DIR      QString("/usr/share/") + qApp->applicationName()
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

QString ProgramPaths::tilesDir()
{
	return dir(TILES_DIR);
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

#else // QT_VERSION < 5

#include <QStandardPaths>

QString ProgramPaths::mapDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(MAP_DIR);
	else
		return QString(QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  MAP_DIR, QStandardPaths::LocateDirectory));
}

QString ProgramPaths::poiDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(POI_DIR);
	else
		return QString(QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  POI_DIR, QStandardPaths::LocateDirectory));
}

QString ProgramPaths::csvDir(bool writable)
{
	if (writable)
		return QDir(QStandardPaths::writableLocation(
		  QStandardPaths::AppDataLocation)).filePath(CSV_DIR);
	else
		return QString(QStandardPaths::locate(QStandardPaths::AppDataLocation,
		  CSV_DIR, QStandardPaths::LocateDirectory));
}

QString ProgramPaths::tilesDir()
{
	return QString(QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
	  TILES_DIR, QStandardPaths::LocateDirectory));
}

QString ProgramPaths::translationsDir()
{
	return QString(QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  TRANSLATIONS_DIR, QStandardPaths::LocateDirectory));
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

#endif // QT_VERSION < 5
