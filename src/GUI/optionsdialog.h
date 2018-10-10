#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "palette.h"
#include "units.h"
#include "config.h"


class ColorBox;
class StyleComboBox;
class OddSpinBox;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QRadioButton;
class PercentSlider;

struct Options {
	// Appearance
	Palette palette;
	int trackWidth;
	int routeWidth;
	Qt::PenStyle trackStyle;
	Qt::PenStyle routeStyle;
	QColor waypointColor;
	QColor poiColor;
	int waypointSize;
	int poiSize;
	int graphWidth;
	QColor sliderColor;
	bool pathAntiAliasing;
	bool graphAntiAliasing;
	int mapOpacity;
	QColor backgroundColor;
	// Map
	bool alwaysShowMap;
#ifdef ENABLE_HIDPI
	bool hidpiMap;
#endif // ENABLE_HIDPI
	// Data
	int elevationFilter;
	int speedFilter;
	int heartRateFilter;
	int cadenceFilter;
	int powerFilter;
	bool outlierEliminate;
	qreal pauseSpeed;
	int pauseInterval;
	bool useReportedSpeed;
	// POI
	int poiRadius;
	// System
	bool useOpenGL;
#ifdef ENABLE_HTTP2
	bool enableHTTP2;
#endif // ENABLE_HTTP2
	int pixmapCache;
	int connectionTimeout;
	// Print/Export
	bool hiresPrint;
	bool printName;
	bool printDate;
	bool printDistance;
	bool printTime;
	bool printMovingTime;
	bool printItemCount;
	bool separateGraphPage;

	Units units;
};

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:
	OptionsDialog(Options *options, QWidget *parent = 0);

public slots:
	void accept();

private:
	QWidget *createMapPage();
	QWidget *createAppearancePage();
	QWidget *createDataPage();
	QWidget *createPOIPage();
	QWidget *createSystemPage();
	QWidget *createExportPage();

	Options *_options;

	// Appearance
	ColorBox *_baseColor;
	QDoubleSpinBox *_colorOffset;
	PercentSlider *_mapOpacity;
	ColorBox *_backgroundColor;
	QSpinBox *_trackWidth;
	StyleComboBox *_trackStyle;
	QSpinBox *_routeWidth;
	StyleComboBox *_routeStyle;
	QCheckBox *_pathAA;
	QSpinBox *_waypointSize;
	ColorBox *_waypointColor;
	QSpinBox *_poiSize;
	ColorBox *_poiColor;
	QSpinBox *_graphWidth;
	ColorBox *_sliderColor;
	QCheckBox *_graphAA;
	// Map
	QCheckBox *_alwaysShowMap;
#ifdef ENABLE_HIDPI
	QRadioButton *_hidpi;
	QRadioButton *_lodpi;
#endif // ENABLE_HIDPI
	// Data
	OddSpinBox *_elevationFilter;
	OddSpinBox *_speedFilter;
	OddSpinBox *_heartRateFilter;
	OddSpinBox *_cadenceFilter;
	OddSpinBox *_powerFilter;
	QCheckBox *_outlierEliminate;
	QDoubleSpinBox *_pauseSpeed;
	QSpinBox *_pauseInterval;
	QRadioButton *_computed;
	QRadioButton *_reported;
	// POI
	QDoubleSpinBox *_poiRadius;
	// System
	QSpinBox *_pixmapCache;
	QSpinBox *_connectionTimeout;
	QCheckBox *_useOpenGL;
#ifdef ENABLE_HTTP2
	QCheckBox *_enableHTTP2;
#endif // ENABLE_HTTP2
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
