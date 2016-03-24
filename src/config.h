#ifndef CONFIG_H
#define CONFIG_H

#include <QtGlobal>

#define APP_NAME        "GPXSee"
#define APP_HOMEPAGE    "http://tumic.wz.cz/gpxsee"
#define APP_VERSION     "2.10"

#define FONT_FAMILY     "Arial"
#define FONT_SIZE       12

#define MAP_FILE        QString("maps.txt")
#define POI_DIR         QString("POI")

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

#define USER_MAP_FILE   USER_DIR + QString("/") + MAP_FILE
#define USER_POI_DIR    USER_DIR + QString("/") + POI_DIR
#define GLOBAL_MAP_FILE GLOBAL_DIR + QString("/") + MAP_FILE
#define GLOBAL_POI_DIR  GLOBAL_DIR + QString("/") + POI_DIR
#define TILES_DIR       USER_DIR + QString("/tiles")

#endif /* CONFIG_H */
