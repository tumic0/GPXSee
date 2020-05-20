#ifndef CONFIG_H
#define CONFIG_H

#include <QtGlobal>

#define APP_NAME        "GPXSee"
#define APP_HOMEPAGE    "http://www.gpxsee.org"

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 1)
#define ENABLE_HTTP2
#endif // QT >= 5.10.1

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
#define ENABLE_HIDPI
#endif // QT >= 5.6

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#define ENABLE_GEOJSON
#endif // QT >= 5.0

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#define ENABLE_TIMEZONES
#endif // QT >= 5.2

#endif /* CONFIG_H */
