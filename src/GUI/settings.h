#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QVariant>
#include <QSettings>

#define SETTINGS_WINDOW     "Window"
#define SETTINGS_SETTINGS   "Settings"
#define SETTINGS_FILE       "File"
#define SETTINGS_MAP        "Map"
#define SETTINGS_GRAPH      "Graph"
#define SETTINGS_POI        "POI"
#define SETTINGS_DATA       "Data"
#define SETTINGS_DEM        "DEM"
#define SETTINGS_POSITION   "Position"
#define SETTINGS_PDF_EXPORT "Export"
#define SETTINGS_PNG_EXPORT "PNGExport"
#define SETTINGS_OPTIONS    "Options"

class Settings
{
public:
	class Setting
	{
	public:
		Setting(const QString &name, const QVariant &defVal)
		  : _name(name), _defVal(defVal) {}

		void write(QSettings &settings, const QVariant &value) const
		{
			if (value != _defVal)
				settings.setValue(_name, value);
		}
		QVariant read(const QSettings &settings) const
		{
			return settings.value(_name, _defVal);
		}

	private:
		QString _name;
		QVariant _defVal;
	};

	class SettingMap
	{
	public:
		SettingMap(const QString &prefix, const QString &key, const QString &value)
		  : _prefix(prefix), _key(key), _value(value) {}

		void write(QSettings &settings, const QMap<QString, QVariantMap> &map) const;
		QMap<QString, QVariantMap> read(QSettings &settings) const;

	private:
		QString _prefix;
		QString _key;
		QString _value;
	};

	class SettingList
	{
	public:
		SettingList(const QString &prefix, const QString &value)
		  : _prefix(prefix), _value(value) {}

		void write(QSettings &settings, const QStringList &list) const;
		QStringList read(QSettings &settings) const;

	private:
		QString _prefix;
		QString _value;
	};


	/* Window */
#ifndef Q_OS_ANDROID
	static const Setting windowGeometry;
	static const Setting windowState;
#endif // Q_OS_ANDROID

	/* Settings */
	static const Setting timeType;
	static const Setting units;
	static const Setting coordinatesFormat;
#ifndef Q_OS_ANDROID
	static const Setting showToolbars;
#endif // Q_OS_ANDROID

	/* File */
#ifndef Q_OS_ANDROID
	static const SettingList recentDataFiles;
#endif // Q_OS_ANDROID

	/* Map */
	static const Setting activeMap;
	static const Setting showMap;
	static const Setting cursorCoordinates;

	/* Graph */
	static const Setting showGraphs;
	static const Setting graphType;
	static const Setting showGrid;
	static const Setting sliderInfo;
#ifdef Q_OS_ANDROID
	static const Setting showGraphTabs;
#endif // Q_OS_ANDROID

	/* POI */
	static const Setting poiIcons;
	static const Setting poiLabels;
	static const Setting showPoi;
	static const Setting poiOverlap;
	static const SettingList disabledPoiFiles;

	/* Data */
	static const Setting tracks;
	static const Setting routes;
	static const Setting waypoints;
	static const Setting areas;
	static const Setting routeWaypoints;
	static const Setting waypointIcons;
	static const Setting waypointLabels;
	static const Setting pathTicks;
	static const Setting positionMarkers;
	static const Setting markerInfo;
	static const Setting useStyles;

	/* DEM */
	static const Setting drawHillShading;

	/* Position */
	static const Setting showPosition;
	static const Setting followPosition;
	static const Setting positionCoordinates;
	static const Setting motionInfo;

	/* PDF export */
	static const Setting pdfOrientation;
	static const Setting pdfSize;
	static const Setting pdfMarginLeft;
	static const Setting pdfMarginTop;
	static const Setting pdfMarginRight;
	static const Setting pdfMarginBottom;
	static const Setting pdfFileName;
	static const Setting pdfResolution;

	/* PNG export */
	static const Setting pngWidth;
	static const Setting pngHeight;
	static const Setting pngMarginLeft;
	static const Setting pngMarginTop;
	static const Setting pngMarginRight;
	static const Setting pngMarginBottom;
	static const Setting pngAntialiasing;
	static const Setting pngFileName;

	/* Options */
	static const Setting paletteColor;
	static const Setting paletteShift;
	static const Setting mapOpacity;
	static const Setting backgroundColor;
	static const Setting crosshairColor;
	static const Setting infoColor;
	static const Setting infoBackground;
	static const Setting trackWidth;
	static const Setting routeWidth;
	static const Setting areaWidth;
	static const Setting trackStyle;
	static const Setting routeStyle;
	static const Setting areaStyle;
	static const Setting areaOpacity;
	static const Setting waypointSize;
	static const Setting waypointColor;
	static const Setting poiSize;
	static const Setting poiColor;
	static const Setting graphWidth;
	static const Setting pathAntiAliasing;
	static const Setting graphAntiAliasing;
	static const Setting elevationFilter;
	static const Setting speedFilter;
	static const Setting heartRateFilter;
	static const Setting cadenceFilter;
	static const Setting powerFilter;
	static const Setting outlierEliminate;
	static const Setting automaticPause;
	static const Setting pauseSpeed;
	static const Setting pauseInterval;
	static const Setting useReportedSpeed;
	static const Setting dataUseDEM;
	static const Setting secondaryElevation;
	static const Setting secondarySpeed;
	static const Setting timeZone;
	static const Setting useSegments;
	static const Setting poiRadius;
	static const Setting demURL;
	static const Setting demAuthentication;
	static const Setting demUsername;
	static const Setting demPassword;
	static const Setting useOpenGL;
	static const Setting enableHTTP2;
	static const Setting pixmapCache;
	static const Setting demCache;
	static const Setting connectionTimeout;
	static const Setting hiresPrint;
	static const Setting printName;
	static const Setting printDate;
	static const Setting printDistance;
	static const Setting printTime;
	static const Setting printMovingTime;
	static const Setting printItemCount;
	static const Setting separateGraphPage;
	static const Setting sliderColor;
	static const Setting outputProjection;
	static const Setting inputProjection;
	static const Setting hidpiMap;
	static const Setting poiPath;
	static const Setting mapsPath;
	static const Setting dataPath;
	static const Setting &positionPlugin();
	static const SettingMap positionPluginParameters;
};

#endif // SETTINGS_H
