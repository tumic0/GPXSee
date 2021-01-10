#ifndef CONFIG_H
#define CONFIG_H

#include <QtGlobal>

#define APP_NAME        "GPXSee"
#define APP_HOMEPAGE    "http://www.gpxsee.org"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define HASH_T uint
#else // QT6
#define HASH_T size_t
#endif // QT6

#endif /* CONFIG_H */
