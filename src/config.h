#ifndef CONFIG_H
#define CONFIG_H

#include <QtGlobal>
#include <QDir>
#include <QApplication>
#include <QString>

#define APP_NAME        "GPXSee"
#define APP_HOMEPAGE    "http://www.gpxsee.org"

#define FONT_FAMILY     "Arial"
#define FONT_SIZE       12 // px

#define MAP_DIR         QString("maps")
#define POI_DIR         QString("POI")
#define CSV_DIR         QString("csv")
#define ELLIPSOID_FILE  QString("ellipsoids.csv")
#define GCS_FILE        QString("gcs.csv")
#define PCS_FILE        QString("pcs.csv")

#if defined(Q_OS_WIN32)
#define USER_DIR        QDir::homePath() + QString("/GPXSee")
#define GLOBAL_DIR      QApplication::applicationDirPath()
#elif defined(Q_OS_MAC)
#define USER_DIR        QDir::homePath() + QString("/.gpxsee")
#define GLOBAL_DIR      QApplication::applicationDirPath() \
                          + QString("/../Resources")
#else
#define USER_DIR        QDir::homePath() + QString("/.gpxsee")
#define GLOBAL_DIR      QString("/usr/share/gpxsee")
#endif

#define USER_CSV_DIR           USER_DIR + QString("/") + CSV_DIR
#define USER_ELLIPSOID_FILE    USER_CSV_DIR + QString("/") + ELLIPSOID_FILE
#define USER_GCS_FILE          USER_CSV_DIR + QString("/") + GCS_FILE
#define USER_PCS_FILE          USER_CSV_DIR + QString("/") + PCS_FILE
#define USER_MAP_DIR           USER_DIR + QString("/") + MAP_DIR
#define USER_POI_DIR           USER_DIR + QString("/") + POI_DIR
#define GLOBAL_CSV_DIR         GLOBAL_DIR + QString("/") + CSV_DIR
#define GLOBAL_ELLIPSOID_FILE  GLOBAL_CSV_DIR + QString("/") + ELLIPSOID_FILE
#define GLOBAL_GCS_FILE        GLOBAL_CSV_DIR + QString("/") + GCS_FILE
#define GLOBAL_PCS_FILE        GLOBAL_CSV_DIR + QString("/") + PCS_FILE
#define GLOBAL_MAP_DIR         GLOBAL_DIR + QString("/") + MAP_DIR
#define GLOBAL_POI_DIR         GLOBAL_DIR + QString("/") + POI_DIR
#define TILES_DIR              USER_DIR + QString("/tiles")
#define TRANSLATIONS_DIR       GLOBAL_DIR + QString("/translations")


#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 1)
#define ENABLE_HTTP2
#endif // QT >= 5.10.1

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
#define ENABLE_HIDPI
#endif // QT >= 5.6

#endif /* CONFIG_H */
