#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "palette.h"
#include "units.h"

class ColorBox;
class StyleComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

struct Options {
	// Appearance
	Palette palette;
	int trackWidth;
	int routeWidth;
	Qt::PenStyle trackStyle;
	Qt::PenStyle routeStyle;
	int graphWidth;
	bool pathAntiAliasing;
	bool graphAntiAliasing;
	// POI
	int poiRadius;
	// System
	bool useOpenGL;
	// Print/Export
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
	QWidget *createAppearancePage();
	QWidget *createPOIPage();
	QWidget *createSystemPage();
	QWidget *createExportPage();

	Options *_options;

	ColorBox *_baseColor;
	QDoubleSpinBox *_colorOffset;
	QSpinBox *_trackWidth;
	StyleComboBox *_trackStyle;
	QSpinBox *_routeWidth;
	StyleComboBox *_routeStyle;
	QCheckBox *_pathAA;
	QSpinBox *_graphWidth;
	QCheckBox *_graphAA;
	QDoubleSpinBox *_poiRadius;
	QCheckBox *_useOpenGL;
	QCheckBox *_name;
	QCheckBox *_date;
	QCheckBox *_distance;
	QCheckBox *_time;
	QCheckBox *_movingTime;
	QCheckBox *_itemCount;
	QCheckBox *_separateGraphPage;
};

#endif // OPTIONSDIALOG_H
