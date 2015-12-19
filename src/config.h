#ifndef CONFIG_H
#define CONFIG_H

#define APP_NAME       "GPXSee"
#define APP_HOMEPAGE   "http://tumic.wz.cz/gpxsee"
#define APP_VERSION    "2.4"

#define FONT_FAMILY    "Arial"
#define FONT_SIZE      12

#if defined(Q_OS_WIN32)
#define APP_DIR        "GPXSee"
#else
#define APP_DIR        ".gpxsee"
#endif
#define POI_DIR        APP_DIR"/POI"
#define TILES_DIR      APP_DIR"/tiles"
#define MAP_LIST_FILE  APP_DIR"/maps.txt"

#endif /* CONFIG_H */
