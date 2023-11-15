#include <QLocale>
#include <QDir>
#include <QPageLayout>
#include <QPageSize>
#include <QGeoPositionInfoSource>
#include "common/config.h"
#include "common/util.h"
#include "data/graph.h"
#include "format.h"
#include "units.h"
#include "timetype.h"
#include "markerinfoitem.h"
#include "timezoneinfo.h"
#include "settings.h"


#define SETTING(varName, name, defVal) \
	const Settings::Setting Settings::varName = Settings::Setting(name, defVal)

#define IMPERIAL_UNITS() \
	(QLocale::system().measurementSystem() == QLocale::ImperialSystem)
#define CWD(filename) \
	QDir::current().filePath(filename)
#define UNITS() \
	(IMPERIAL_UNITS() ? Imperial : Metric)
#define TIMEZONE() \
	QVariant::fromValue(TimeZoneInfo())
#define POI_RADIUS() \
	(int)(IMPERIAL_UNITS() ? MIINM : KMINM)
#define PAPER_SIZE() \
	(IMPERIAL_UNITS() \
	  ? QPageSize::PageSizeId::Letter \
	  : QPageSize::PageSizeId::A4)

#ifdef Q_OS_ANDROID
#define PIXMAP_CACHE 256
#define DEM_CACHE    128
#else // Q_OS_ANDROID
#define PIXMAP_CACHE 512
#define DEM_CACHE    256
#endif // Q_OS_ANDROID


static QString defaultPlugin()
{
	QString source;

	QGeoPositionInfoSource *ps = QGeoPositionInfoSource::createDefaultSource(0);
	if (ps) {
		source = ps->sourceName();
		delete ps;
	}

	return source;
}

QMap<QString, QVariantMap> Settings::SettingMap::read(QSettings &settings) const
{
	QMap<QString, QVariantMap> map;
	int size = settings.beginReadArray(_prefix);

	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		map.insert(settings.value(_key).toString(),
		  settings.value(_value).toMap());
	}
	settings.endArray();

	return map;
}

void Settings::SettingMap::write(QSettings &settings,
  const QMap<QString, QVariantMap> &map) const
{
	int index = 0;

	if (map.isEmpty())
		return;

	settings.beginWriteArray(_prefix);

	for (QMap<QString, QVariantMap>::const_iterator it = map.constBegin();
	  it != map.constEnd(); ++it) {
		if (!it.value().isEmpty()) {
			settings.setArrayIndex(index++);
			settings.setValue(_key, it.key());
			settings.setValue(_value, it.value());
		}
	}

	settings.endArray();
}

QStringList Settings::SettingList::read(QSettings &settings) const
{
	QStringList list;
	int size = settings.beginReadArray(_prefix);

	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		list.append(settings.value(_value).toString());
	}
	settings.endArray();

	return list;
}

void Settings::SettingList::write(QSettings &settings,
  const QStringList &list) const
{
	if (list.isEmpty())
		return;

	settings.beginWriteArray(_prefix);

	for (int i = 0; i < list.size(); i++) {
		settings.setArrayIndex(i);
		settings.setValue(_value, list.at(i));
	}

	settings.endArray();
}

const Settings::Setting &Settings::positionPlugin()
{
	static Setting s("positionPlugin", defaultPlugin());
	return s;
}

/* Window */
#ifndef Q_OS_ANDROID
SETTING(windowGeometry,      "geometry",               QByteArray()           );
SETTING(windowState,         "state",                  QByteArray()           );
#endif // Q_OS_ANDROID

/* Settings */
SETTING(timeType,            "timeType",               Total                  );
SETTING(units,               "units",                  UNITS()                );
SETTING(coordinatesFormat,   "coordinates",            DecimalDegrees         );
#ifndef Q_OS_ANDROID
SETTING(showToolbars,        "toolbar",                true                   );
#endif // Q_OS_ANDROID

/* Map */
SETTING(activeMap,           "map",                    "Open Street Map"      );
SETTING(showMap,             "show",                   true                   );
SETTING(cursorCoordinates,   "coordinates",            false                  );

/* Graph */
SETTING(showGraphs,          "show",                   true                   );
SETTING(graphType,           "type",                   Distance               );
SETTING(showGrid,            "grid",                   true                   );
SETTING(sliderInfo,          "sliderInfo",             true                   );
#ifdef Q_OS_ANDROID
SETTING(showGraphTabs,       "tabs",                   true                   );
#endif // Q_OS_ANDROID

/* POI */
SETTING(poiIcons,            "icons",                  true                   );
SETTING(poiLabels,           "labels",                 true                   );
SETTING(showPoi,             "show",                   false                  );
SETTING(poiOverlap,          "overlap",                false                  );

/* Data */
SETTING(tracks,              "tracks",                 true                   );
SETTING(routes,              "routes",                 true                   );
SETTING(waypoints,           "waypoints",              true                   );
SETTING(areas,               "areas",                  true                   );
SETTING(routeWaypoints,      "routeWaypoints",         true                   );
SETTING(waypointIcons,       "waypointIcons",          false                  );
SETTING(waypointLabels,      "waypointLabels",         true                   );
SETTING(pathTicks,           "pathTicks",              false                  );
SETTING(positionMarkers,     "positionMarkers",        true                   );
SETTING(markerInfo,          "markerInfo",             MarkerInfoItem::None   );
SETTING(useStyles,           "styles",                 true                   );

/* Position */
SETTING(showPosition,        "show",                   false                  );
SETTING(followPosition,      "follow",                 true                   );
SETTING(positionCoordinates, "coordinates",            true                   );
SETTING(motionInfo,          "motionInfo",             true                   );

/* PDF export */
SETTING(pdfOrientation,      "orientation", QPageLayout::Orientation::Portrait);
SETTING(pdfSize,             "size",                   PAPER_SIZE()           );
SETTING(pdfMarginLeft,       "marginLeft",             5                      );
SETTING(pdfMarginTop,        "marginTop",              5                      );
SETTING(pdfMarginRight,      "marginRight",            5                      );
SETTING(pdfMarginBottom,     "marginBottom",           5                      );
SETTING(pdfFileName,         "fileName",               CWD("export.pdf")      );
SETTING(pdfResolution,       "resolution",             600                    );

/* PNG export */
SETTING(pngWidth,            "width",                  600                    );
SETTING(pngHeight,           "height",                 800                    );
SETTING(pngMarginLeft,       "marginLeft",             5                      );
SETTING(pngMarginTop,        "marginTop",              5                      );
SETTING(pngMarginRight,      "marginRight",            5                      );
SETTING(pngMarginBottom,     "marginBottom",           5                      );
SETTING(pngAntialiasing,     "antialiasing",           true                   );
SETTING(pngFileName,         "fileName",               CWD("export.png")      );

/* Options */
SETTING(paletteColor,        "paletteColor",           QColor(Qt::blue)       );
SETTING(paletteShift,        "paletteShift",           0.62                   );
SETTING(mapOpacity,          "mapOpacity",             100                    );
SETTING(backgroundColor,     "backgroundColor",        QColor(Qt::white)      );
SETTING(crosshairColor,      "crosshairColor",         QColor(Qt::red)        );
SETTING(infoColor,           "infoColor",              QColor(Qt::black)      );
SETTING(infoBackground,      "infoBackground",         false                  );
SETTING(trackWidth,          "trackWidth",             3                      );
SETTING(routeWidth,          "routeWidth",             3                      );
SETTING(areaWidth,           "areaWidth",              2                      );
SETTING(trackStyle,          "trackStyle",             (int)Qt::SolidLine     );
SETTING(routeStyle,          "routeStyle",             (int)Qt::DotLine       );
SETTING(areaStyle,           "areaStyle",              (int)Qt::SolidLine     );
SETTING(areaOpacity,         "areaOpacity",            50                     );
SETTING(waypointSize,        "waypointSize",           8                      );
SETTING(waypointColor,       "waypointColor",          QColor(Qt::black)      );
SETTING(poiSize,             "poiSize",                8                      );
SETTING(poiColor,            "poiColor",               QColor(Qt::black)      );
SETTING(graphWidth,          "graphWidth",             1                      );
SETTING(pathAntiAliasing,    "pathAntiAliasing",       true                   );
SETTING(graphAntiAliasing,   "graphAntiAliasing",      true                   );
SETTING(elevationFilter,     "elevationFilter",        3                      );
SETTING(speedFilter,         "speedFilter",            5                      );
SETTING(heartRateFilter,     "heartrateFilter",        3                      );
SETTING(cadenceFilter,       "cadenceFilter",          3                      );
SETTING(powerFilter,         "powerFilter",            3                      );
SETTING(outlierEliminate,    "outlierEliminate",       true                   );
SETTING(automaticPause,      "automaticPause",         true                   );
SETTING(pauseSpeed,          "pauseSpeed",             0.5                    );
SETTING(pauseInterval,       "pauseInterval",          10                     );
SETTING(useReportedSpeed,    "useReportedSpeed",       false                  );
SETTING(dataUseDEM,          "dataUseDEM",             false                  );
SETTING(secondaryElevation,  "showSecondaryElevation", false                  );
SETTING(secondarySpeed,      "showSecondarySpeed",     false                  );
SETTING(timeZone,            "timeZone",               TIMEZONE()             );
SETTING(useSegments,         "useSegments",            true                   );
SETTING(poiRadius,           "poiRadius",              POI_RADIUS()           );
SETTING(demURL,              "demURL",                 DEM_TILES_URL          );
SETTING(demAuthentication,   "demAuthentication",      false                  );
SETTING(demUsername,         "demUsername",            ""                     );
SETTING(demPassword,         "demPassword",            ""                     );
SETTING(useOpenGL,           "useOpenGL",              false                  );
SETTING(enableHTTP2,         "enableHTTP2",            true                   );
SETTING(pixmapCache,         "pixmapCache",            PIXMAP_CACHE           );
SETTING(demCache,            "demCache",               DEM_CACHE              );
SETTING(connectionTimeout,   "connectionTimeout",      30                     );
SETTING(hiresPrint,          "hiresPrint",             false                  );
SETTING(printName,           "printName",              true                   );
SETTING(printDate,           "printDate",              true                   );
SETTING(printDistance,       "printDistance",          true                   );
SETTING(printTime,           "printTime",              true                   );
SETTING(printMovingTime,     "printMovingTime",        false                  );
SETTING(printItemCount,      "printItemCount",         true                   );
SETTING(separateGraphPage,   "separateGraphPage",      false                  );
SETTING(sliderColor,         "sliderColor",            QColor(Qt::red)        );
SETTING(outputProjection,    "outputProjection",       3856                   );
SETTING(inputProjection,     "inputProjection",        4326                   );
SETTING(hidpiMap,            "HiDPIMap",               true                   );
SETTING(poiPath,             "poiPath",                ""                     );
SETTING(mapsPath,            "mapsPath",               ""                     );
SETTING(dataPath,            "dataPath",               ""                     );

const Settings::SettingMap Settings::positionPluginParameters
  = Settings::SettingMap("pluginParameters", "plugin", "parameters");

const Settings::SettingList Settings::disabledPoiFiles
  = Settings::SettingList("disabled", "file");

#ifndef Q_OS_ANDROID
const Settings::SettingList Settings::recentDataFiles
  = Settings::SettingList("recent", "file");
#endif // Q_OS_ANDROID
