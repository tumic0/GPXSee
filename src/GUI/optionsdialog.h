#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QtGlobal>
#include <QDialog>
#include <QPainter>
#include "palette.h"
#include "units.h"
#include "timezoneinfo.h"

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QLineEdit;
class ColorBox;
class StyleComboBox;
class OddSpinBox;
class PercentSlider;
class ProjectionComboBox;
class DirSelectWidget;
class AuthenticationWidget;
class PluginParameters;

struct Options {
	// Appearance
	Palette palette;
	int trackWidth;
	int routeWidth;
	int areaWidth;
	Qt::PenStyle trackStyle;
	Qt::PenStyle routeStyle;
	Qt::PenStyle areaStyle;
	int areaOpacity;
	QColor waypointColor;
	QColor poiColor;
	int waypointSize;
	int poiSize;
	int graphWidth;
	QColor sliderColor;
	bool pathAntiAliasing;
	bool graphAntiAliasing;
	int mapOpacity;
	int overlayOpacity;
	QPainter::CompositionMode overlayMode;
	QColor backgroundColor;
	QColor crosshairColor;
	QColor infoColor;
	bool infoBackground;
	// Map
	int outputProjection;
	int inputProjection;
	bool hidpiMap;
	// Data
	int elevationFilter;
	int speedFilter;
	int heartRateFilter;
	int cadenceFilter;
	int powerFilter;
	bool outlierEliminate;
	bool automaticPause;
	qreal pauseSpeed;
	int pauseInterval;
	bool useReportedSpeed;
	bool dataUseDEM;
	bool showSecondaryElevation;
	bool showSecondarySpeed;
	TimeZoneInfo timeZone;
	bool useSegments;
	// POI
	int poiRadius;
	// DEM
	QString demURL;
	QString demUsername;
	QString demPassword;
	bool demAuthorization;
	// Position
	QString plugin;
	QMap<QString, QVariantMap> pluginParams;
	// System
	bool useOpenGL;
	bool enableHTTP2;
	int pixmapCache;
	int demCache;
	int connectionTimeout;
	QString dataPath;
	QString mapsPath;
	QString poiPath;
	// Print/Export
	bool hiresPrint;
	bool printName;
	bool printDate;
	bool printDistance;
	bool printTime;
	bool printMovingTime;
	bool printItemCount;
	bool separateGraphPage;
};

class OptionsDialog : public QDialog
{
	Q_OBJECT

public slots:
	void accept();

public:
	OptionsDialog(Options &options, Units units, QWidget *parent = 0);

private slots:
	void automaticPauseDetectionSet(bool set);

private:
	QWidget *createMapPage();
	QWidget *createAppearancePage();
	QWidget *createDataPage();
	QWidget *createPOIPage();
	QWidget *createSystemPage();
	QWidget *createExportPage();
	QWidget *createDEMPage();
	QWidget *createPositionPage();

	Options &_options;

	Units _units;
	// Appearance
	ColorBox *_baseColor;
	PercentSlider *_colorOffset;
	PercentSlider *_mapOpacity;
	PercentSlider *_overlayOpacity;
	QSpinBox *_overlayMode;
	ColorBox *_backgroundColor;
	ColorBox *_crosshairColor;
	ColorBox *_infoColor;
	QCheckBox *_infoBackground;
	QSpinBox *_trackWidth;
	StyleComboBox *_trackStyle;
	QSpinBox *_routeWidth;
	StyleComboBox *_routeStyle;
	QSpinBox *_areaWidth;
	StyleComboBox *_areaStyle;
	PercentSlider *_areaOpacity;
	QCheckBox *_pathAA;
	QSpinBox *_waypointSize;
	ColorBox *_waypointColor;
	QSpinBox *_poiSize;
	ColorBox *_poiColor;
	QSpinBox *_graphWidth;
	ColorBox *_sliderColor;
	QCheckBox *_graphAA;
	// Map
	ProjectionComboBox *_outputProjection;
	ProjectionComboBox *_inputProjection;
	QRadioButton *_hidpi;
	QRadioButton *_lodpi;
	// Data
	OddSpinBox *_elevationFilter;
	OddSpinBox *_speedFilter;
	OddSpinBox *_heartRateFilter;
	OddSpinBox *_cadenceFilter;
	OddSpinBox *_powerFilter;
	QCheckBox *_outlierEliminate;
	QRadioButton *_automaticPause;
	QRadioButton *_manualPause;
	QDoubleSpinBox *_pauseSpeed;
	QSpinBox *_pauseInterval;
	QRadioButton *_computedSpeed;
	QRadioButton *_reportedSpeed;
	QRadioButton *_dataGPSElevation;
	QRadioButton *_dataDEMElevation;
	QCheckBox *_showSecondaryElevation;
	QCheckBox *_showSecondarySpeed;
	QRadioButton *_utcZone;
	QRadioButton *_systemZone;
	QRadioButton *_customZone;
	QComboBox *_timeZone;
	QCheckBox *_useSegments;
	// POI
	QDoubleSpinBox *_poiRadius;
	// DEM
	QLineEdit *_demURL;
	AuthenticationWidget *_demAuth;
	// Position
	QComboBox *_positionPlugin;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	PluginParameters *_pluginParameters;
#endif // QT 5.14
	// System
	QSpinBox *_pixmapCache;
	QSpinBox *_demCache;
	QSpinBox *_connectionTimeout;
	QCheckBox *_useOpenGL;
	QCheckBox *_enableHTTP2;
	DirSelectWidget *_dataPath;
	DirSelectWidget *_mapsPath;
	DirSelectWidget *_poiPath;
	// Print/Export
	QRadioButton *_wysiwyg;
	QRadioButton *_hires;
	QCheckBox *_name;
	QCheckBox *_date;
	QCheckBox *_distance;
	QCheckBox *_time;
	QCheckBox *_movingTime;
	QCheckBox *_itemCount;
	QCheckBox *_separateGraphPage;
};

#endif // OPTIONSDIALOG_H
