#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QTabWidget>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>
#include <QSysInfo>
#include "icons.h"
#include "colorbox.h"
#include "stylecombobox.h"
#include "oddspinbox.h"
#include "percentslider.h"
#include "optionsdialog.h"


#define MENU_MARGIN 20
#define MENU_ICON_SIZE 32

#ifdef Q_OS_MAC
static QFrame *line()
{
	QFrame *l = new QFrame();
	l->setFrameShape(QFrame::HLine);
	l->setFrameShadow(QFrame::Sunken);

	return l;
}
#endif

QWidget *OptionsDialog::createMapPage()
{
	_alwaysShowMap = new QCheckBox(tr("Always show the map"));
	_alwaysShowMap->setChecked(_options->alwaysShowMap);
	_alwaysShowMap->setToolTip("<p>" +
	  tr("Show the map even when no files are loaded.") + "</p>");

#ifdef ENABLE_HIDPI
	_hidpi = new QRadioButton(tr("High-resolution"));
	_lodpi = new QRadioButton(tr("Standard"));
	if (_options->hidpiMap)
		_hidpi->setChecked(true);
	else
		_lodpi->setChecked(true);
	QLabel *lhi = new QLabel(tr("Non-HiDPI maps are loaded as HiDPI maps. "
	  "The map is sharp but map objects are small/hard to read."));
	QLabel *llo = new QLabel(tr("Non-HiDPI maps are loaded such as they are. "
	  "Map objects have the expected size but the map is blurry."));
	QFont f = lhi->font();
	f.setPointSize(f.pointSize() - 1);
	lhi->setWordWrap(true);
	llo->setWordWrap(true);
	lhi->setFont(f);
	llo->setFont(f);
#endif // ENABLE_HIDPI

	QFormLayout *showMapLayout = new QFormLayout();
	showMapLayout->addWidget(_alwaysShowMap);

	QWidget *mapTab = new QWidget();
	QVBoxLayout *mapTabLayout = new QVBoxLayout();
	mapTabLayout->addLayout(showMapLayout);
	mapTabLayout->addStretch();
	mapTab->setLayout(mapTabLayout);

#ifdef ENABLE_HIDPI
	QVBoxLayout *hidpiTabLayout = new QVBoxLayout();
	hidpiTabLayout->addWidget(_lodpi);
	hidpiTabLayout->addWidget(llo);
	hidpiTabLayout->addSpacing(10);
	hidpiTabLayout->addWidget(_hidpi);
	hidpiTabLayout->addWidget(lhi);
	hidpiTabLayout->addStretch();

	QWidget *hidpiTab = new QWidget();
	hidpiTab->setLayout(hidpiTabLayout);
#endif // ENABLE_HIDPI

	QTabWidget *mapPage = new QTabWidget();
	mapPage->addTab(mapTab, tr("General"));
#ifdef ENABLE_HIDPI
	mapPage->addTab(hidpiTab, tr("HiDPI display mode"));
#endif // ENABLE_HIDPI

	return mapPage;
}

QWidget *OptionsDialog::createAppearancePage()
{
	// Paths
	_baseColor = new ColorBox();
	_baseColor->setColor(_options->palette.color());
	_colorOffset = new QDoubleSpinBox();
	_colorOffset->setMinimum(0);
	_colorOffset->setMaximum(1.0);
	_colorOffset->setSingleStep(0.01);
	_colorOffset->setValue(_options->palette.shift());
	QFormLayout *paletteLayout = new QFormLayout();
	paletteLayout->addRow(tr("Base color:"), _baseColor);
	paletteLayout->addRow(tr("Palette shift:"), _colorOffset);
#ifndef Q_OS_MAC
	QGroupBox *colorBox = new QGroupBox(tr("Colors"));
	colorBox->setLayout(paletteLayout);
#endif // Q_OS_MAC

	_trackWidth = new QSpinBox();
	_trackWidth->setValue(_options->trackWidth);
	_trackWidth->setMinimum(1);
	_trackStyle = new StyleComboBox();
	_trackStyle->setValue(_options->trackStyle);
	QFormLayout *trackLayout = new QFormLayout();
#ifdef Q_OS_MAC
	trackLayout->addRow(tr("Track width:"), _trackWidth);
	trackLayout->addRow(tr("Track style:"), _trackStyle);
#else // Q_OS_MAC
	trackLayout->addRow(tr("Width:"), _trackWidth);
	trackLayout->addRow(tr("Style:"), _trackStyle);
	QGroupBox *trackBox = new QGroupBox(tr("Tracks"));
	trackBox->setLayout(trackLayout);
#endif // Q_OS_MAC

	_routeWidth = new QSpinBox();
	_routeWidth->setValue(_options->routeWidth);
	_routeWidth->setMinimum(1);
	_routeStyle = new StyleComboBox();
	_routeStyle->setValue(_options->routeStyle);
	QFormLayout *routeLayout = new QFormLayout();
#ifdef Q_OS_MAC
	routeLayout->addRow(tr("Route width:"), _routeWidth);
	routeLayout->addRow(tr("Route style:"), _routeStyle);
#else // Q_OS_MAC
	routeLayout->addRow(tr("Width:"), _routeWidth);
	routeLayout->addRow(tr("Style:"), _routeStyle);
	QGroupBox *routeBox = new QGroupBox(tr("Routes"));
	routeBox->setLayout(routeLayout);
#endif // Q_OS_MAC

	_pathAA = new QCheckBox(tr("Use anti-aliasing"));
	_pathAA->setChecked(_options->pathAntiAliasing);
	QFormLayout *pathAALayout = new QFormLayout();
	pathAALayout->addWidget(_pathAA);

	QWidget *pathTab = new QWidget();
	QVBoxLayout *pathTabLayout = new QVBoxLayout();
#ifdef Q_OS_MAC
	pathTabLayout->addLayout(paletteLayout);
	pathTabLayout->addWidget(line());
	pathTabLayout->addLayout(trackLayout);
	pathTabLayout->addWidget(line());
	pathTabLayout->addLayout(routeLayout);
	pathTabLayout->addWidget(line());
#else // Q_OS_MAC
	pathTabLayout->addWidget(colorBox);
	pathTabLayout->addWidget(trackBox);
	pathTabLayout->addWidget(routeBox);
#endif // Q_OS_MAC
	pathTabLayout->addLayout(pathAALayout);
	pathTabLayout->addStretch();
	pathTab->setLayout(pathTabLayout);


	// Waypoints
	_waypointSize = new QSpinBox();
	_waypointSize->setMinimum(1);
	_waypointSize->setValue(_options->waypointSize);
	_waypointColor = new ColorBox();
	_waypointColor->setColor(_options->waypointColor);
	QFormLayout *waypointLayout = new QFormLayout();
#ifdef Q_OS_MAC
	waypointLayout->addRow(tr("Waypoint color:"), _waypointColor);
	waypointLayout->addRow(tr("Waypoint size:"), _waypointSize);
#else // Q_OS_MAC
	waypointLayout->addRow(tr("Color:"), _waypointColor);
	waypointLayout->addRow(tr("Size:"), _waypointSize);
	QGroupBox *waypointBox = new QGroupBox(tr("Waypoints"));
	waypointBox->setLayout(waypointLayout);
#endif // Q_OS_MAC

	_poiSize = new QSpinBox();
	_poiSize->setMinimum(1);
	_poiSize->setValue(_options->poiSize);
	_poiColor = new ColorBox();
	_poiColor->setColor(_options->poiColor);
	QFormLayout *poiLayout = new QFormLayout();
#ifdef Q_OS_MAC
	poiLayout->addRow(tr("POI color:"), _poiColor);
	poiLayout->addRow(tr("POI size:"), _poiSize);
#else // Q_OS_MAC
	poiLayout->addRow(tr("Color:"), _poiColor);
	poiLayout->addRow(tr("Size:"), _poiSize);
	QGroupBox *poiBox = new QGroupBox(tr("POIs"));
	poiBox->setLayout(poiLayout);
#endif // Q_OS_MAC

	QWidget *pointTab = new QWidget();
	QVBoxLayout *pointTabLayout = new QVBoxLayout();
#ifdef Q_OS_MAC
	pointTabLayout->addLayout(waypointLayout);
	pointTabLayout->addWidget(line());
	pointTabLayout->addLayout(poiLayout);
#else // Q_OS_MAC
	pointTabLayout->addWidget(waypointBox);
	pointTabLayout->addWidget(poiBox);
#endif // Q_OS_MAC
	pointTabLayout->addStretch();
	pointTab->setLayout(pointTabLayout);


	// Graphs
	_sliderColor = new ColorBox();
	_sliderColor->setColor(_options->sliderColor);
	_graphWidth = new QSpinBox();
	_graphWidth->setValue(_options->graphWidth);
	_graphWidth->setMinimum(1);

	QFormLayout *graphLayout = new QFormLayout();
	graphLayout->addRow(tr("Line width:"), _graphWidth);
	graphLayout->addRow(tr("Slider color:"), _sliderColor);

	_graphAA = new QCheckBox(tr("Use anti-aliasing"));
	_graphAA->setChecked(_options->graphAntiAliasing);
	QFormLayout *graphAALayout = new QFormLayout();
	graphAALayout->addWidget(_graphAA);

	QWidget *graphTab = new QWidget();
	QVBoxLayout *graphTabLayout = new QVBoxLayout();
	graphTabLayout->addLayout(graphLayout);
	graphTabLayout->addLayout(graphAALayout);
	graphTabLayout->addStretch();
	graphTab->setLayout(graphTabLayout);


	// Map
	_mapOpacity = new PercentSlider();
	_mapOpacity->setValue(_options->mapOpacity);
	_backgroundColor = new ColorBox();
	_backgroundColor->setColor(_options->backgroundColor);
	_backgroundColor->enableAlphaChannel(false);
	QFormLayout *mapLayout = new QFormLayout();
	mapLayout->addRow(tr("Background color:"), _backgroundColor);
	mapLayout->addRow(tr("Map opacity:"), _mapOpacity);

	QWidget *mapTab = new QWidget();
	QVBoxLayout *mapTabLayout = new QVBoxLayout();
	mapTabLayout->addLayout(mapLayout);
	mapTabLayout->addStretch();
	mapTab->setLayout(mapTabLayout);


	QTabWidget *appearancePage = new QTabWidget();
	appearancePage->addTab(pathTab, tr("Paths"));
	appearancePage->addTab(pointTab, tr("Points"));
	appearancePage->addTab(graphTab, tr("Graphs"));
	appearancePage->addTab(mapTab, tr("Map"));

	return appearancePage;
}

QWidget *OptionsDialog::createDataPage()
{
	QString filterToolTip = tr("Moving average window size");

	_elevationFilter = new OddSpinBox();
	_elevationFilter->setValue(_options->elevationFilter);
	_elevationFilter->setToolTip(filterToolTip);
	_speedFilter = new OddSpinBox();
	_speedFilter->setValue(_options->speedFilter);
	_speedFilter->setToolTip(filterToolTip);
	_heartRateFilter = new OddSpinBox();
	_heartRateFilter->setValue(_options->heartRateFilter);
	_heartRateFilter->setToolTip(filterToolTip);
	_cadenceFilter = new OddSpinBox();
	_cadenceFilter->setValue(_options->cadenceFilter);
	_cadenceFilter->setToolTip(filterToolTip);
	_powerFilter = new OddSpinBox();
	_powerFilter->setValue(_options->powerFilter);
	_powerFilter->setToolTip(filterToolTip);

	QFormLayout *smoothLayout = new QFormLayout();
	smoothLayout->addRow(tr("Elevation:"), _elevationFilter);
	smoothLayout->addRow(tr("Speed:"), _speedFilter);
	smoothLayout->addRow(tr("Heart rate:"), _heartRateFilter);
	smoothLayout->addRow(tr("Cadence:"), _cadenceFilter);
	smoothLayout->addRow(tr("Power:"), _powerFilter);
#ifndef Q_OS_MAC
	QGroupBox *smoothBox = new QGroupBox(tr("Smoothing"));
	smoothBox->setLayout(smoothLayout);
#endif // Q_OS_MAC

	_outlierEliminate = new QCheckBox(tr("Eliminate GPS outliers"));
	_outlierEliminate->setChecked(_options->outlierEliminate);

	QFormLayout *outlierLayout = new QFormLayout();
	outlierLayout->addWidget(_outlierEliminate);
#ifndef Q_OS_MAC
	QGroupBox *outlierBox = new QGroupBox(tr("Outlier elimination"));
	outlierBox->setLayout(outlierLayout);
#endif // Q_OS_MAC

	QWidget *filterTab = new QWidget();
	QVBoxLayout *filterTabLayout = new QVBoxLayout();
#ifdef Q_OS_MAC
	filterTabLayout->addWidget(new QLabel(tr("Smoothing:")));
	filterTabLayout->addLayout(smoothLayout);
	filterTabLayout->addWidget(line());
	filterTabLayout->addLayout(outlierLayout);
#else // Q_OS_MAC
	filterTabLayout->addWidget(smoothBox);
	filterTabLayout->addWidget(outlierBox);
#endif // Q_OS_MAC
	filterTabLayout->addStretch();
	filterTab->setLayout(filterTabLayout);


	_pauseSpeed = new QDoubleSpinBox();
	_pauseSpeed->setDecimals(1);
	_pauseSpeed->setSingleStep(0.1);
	_pauseSpeed->setMinimum(0.1);
	if (_options->units == Imperial) {
		_pauseSpeed->setValue(_options->pauseSpeed * MS2MIH);
		_pauseSpeed->setSuffix(UNIT_SPACE + tr("mi/h"));
	} else if (_options->units == Nautical) {
		_pauseSpeed->setValue(_options->pauseSpeed * MS2KN);
		_pauseSpeed->setSuffix(UNIT_SPACE + tr("kn"));
	} else {
		_pauseSpeed->setValue(_options->pauseSpeed * MS2KMH);
		_pauseSpeed->setSuffix(UNIT_SPACE + tr("km/h"));
	}
	_pauseInterval = new QSpinBox();
	_pauseInterval->setMinimum(1);
	_pauseInterval->setSuffix(UNIT_SPACE + tr("s"));
	_pauseInterval->setValue(_options->pauseInterval);

	QFormLayout *pauseLayout = new QFormLayout();
	pauseLayout->addRow(tr("Minimal speed:"), _pauseSpeed);
	pauseLayout->addRow(tr("Minimal duration:"), _pauseInterval);

	QWidget *pauseTab = new QWidget();
	pauseTab->setLayout(pauseLayout);


	_computed = new QRadioButton(tr("Computed from distance/time"));
	_reported = new QRadioButton(tr("Recorded by device"));
	if (_options->useReportedSpeed)
		_reported->setChecked(true);
	else
		_computed->setChecked(true);

	QFormLayout *sourceLayout = new QFormLayout();
	sourceLayout->addWidget(_computed);
	sourceLayout->addWidget(_reported);

	QWidget *sourceTab = new QWidget();
	sourceTab->setLayout(sourceLayout);


	QTabWidget *filterPage = new QTabWidget();
	filterPage->addTab(filterTab, tr("Filtering"));
	filterPage->addTab(pauseTab, tr("Pause detection"));
	filterPage->addTab(sourceTab, tr("Speed"));

	return filterPage;
}

QWidget *OptionsDialog::createPOIPage()
{
	_poiRadius = new QDoubleSpinBox();
	_poiRadius->setSingleStep(1);
	_poiRadius->setDecimals(1);
	if (_options->units == Imperial) {
		_poiRadius->setValue(_options->poiRadius / MIINM);
		_poiRadius->setSuffix(UNIT_SPACE + tr("mi"));
	} else if (_options->units == Nautical) {
		_poiRadius->setValue(_options->poiRadius / NMIINM);
		_poiRadius->setSuffix(UNIT_SPACE + tr("nmi"));
	} else {
		_poiRadius->setValue(_options->poiRadius / KMINM);
		_poiRadius->setSuffix(UNIT_SPACE + tr("km"));
	}

	QFormLayout *poiLayout = new QFormLayout();
	poiLayout->addRow(tr("POI radius:"), _poiRadius);

	QWidget *poiTab = new QWidget();
	poiTab->setLayout(poiLayout);

	QTabWidget *poiPage = new QTabWidget();
	poiPage->addTab(poiTab, tr("POI"));

	return poiPage;
}

QWidget *OptionsDialog::createExportPage()
{
	_wysiwyg = new QRadioButton(tr("WYSIWYG"));
	_hires = new QRadioButton(tr("High-Resolution"));
	if (_options->hiresPrint)
		_hires->setChecked(true);
	else
		_wysiwyg->setChecked(true);
	QLabel *lw = new QLabel(tr("The printed area is approximately the display"
	  " area. The map zoom level does not change."));
	QLabel *lh = new QLabel(tr("The zoom level will be changed so that"
	  " the whole content (tracks/waypoints) fits to the printed area and"
	  " the map resolution is as close as possible to the print resolution."));
	QFont f = lw->font();
	f.setPointSize(f.pointSize() - 1);
	lw->setWordWrap(true);
	lh->setWordWrap(true);
	lw->setFont(f);
	lh->setFont(f);

	QVBoxLayout *modeTabLayout = new QVBoxLayout();
	modeTabLayout->addWidget(_wysiwyg);
	modeTabLayout->addWidget(lw);
	modeTabLayout->addSpacing(10);
	modeTabLayout->addWidget(_hires);
	modeTabLayout->addWidget(lh);
	modeTabLayout->addStretch();

	QWidget *modeTab = new QWidget();
	modeTab->setLayout(modeTabLayout);


	_name = new QCheckBox(tr("Name"));
	_name->setChecked(_options->printName);
	_date = new QCheckBox(tr("Date"));
	_date->setChecked(_options->printDate);
	_distance = new QCheckBox(tr("Distance"));
	_distance->setChecked(_options->printDistance);
	_time = new QCheckBox(tr("Time"));
	_time->setChecked(_options->printTime);
	_movingTime = new QCheckBox(tr("Moving time"));
	_movingTime->setChecked(_options->printMovingTime);
	_itemCount = new QCheckBox(tr("Item count (>1)"));
	_itemCount->setChecked(_options->printItemCount);

	QFormLayout *headerTabLayout = new QFormLayout();
	headerTabLayout->addWidget(_name);
	headerTabLayout->addWidget(_date);
	headerTabLayout->addWidget(_distance);
	headerTabLayout->addWidget(_time);
	headerTabLayout->addWidget(_movingTime);
	headerTabLayout->addItem(new QSpacerItem(10, 10));
	headerTabLayout->addWidget(_itemCount);
	QWidget *headerTab = new QWidget();
	headerTab->setLayout(headerTabLayout);


	_separateGraphPage = new QCheckBox(tr("Separate graph page"));
	_separateGraphPage->setChecked(_options->separateGraphPage);

	QFormLayout *graphTabLayout = new QFormLayout();
	graphTabLayout->addWidget(_separateGraphPage);
	QWidget *graphTab = new QWidget();
	graphTab->setLayout(graphTabLayout);


	QTabWidget *exportPage = new QTabWidget();
	exportPage->addTab(modeTab, tr("Print mode"));
	exportPage->addTab(headerTab, tr("Header"));
	exportPage->addTab(graphTab, tr("Graphs"));

	return exportPage;
}

QWidget *OptionsDialog::createSystemPage()
{
	_useOpenGL = new QCheckBox(tr("Use OpenGL"));
	_useOpenGL->setChecked(_options->useOpenGL);
#ifdef ENABLE_HTTP2
	_enableHTTP2 = new QCheckBox(tr("Enable HTTP/2"));
	_enableHTTP2->setChecked(_options->enableHTTP2);
#endif // ENABLE_HTTP2

	_pixmapCache = new QSpinBox();
	_pixmapCache->setMinimum(16);
	_pixmapCache->setMaximum(1024);
	_pixmapCache->setSuffix(UNIT_SPACE + tr("MB"));
	_pixmapCache->setValue(_options->pixmapCache);

	_connectionTimeout = new QSpinBox();
	_connectionTimeout->setMinimum(30);
	_connectionTimeout->setMaximum(120);
	_connectionTimeout->setSuffix(UNIT_SPACE + tr("s"));
	_connectionTimeout->setValue(_options->connectionTimeout);

	QFormLayout *formLayout = new QFormLayout();
	formLayout->addRow(tr("Image cache size:"), _pixmapCache);
	formLayout->addRow(tr("Connection timeout:"), _connectionTimeout);

	QFormLayout *checkboxLayout = new QFormLayout();
#ifdef ENABLE_HTTP2
	checkboxLayout->addWidget(_enableHTTP2);
#endif // ENABLE_HTTP2
	checkboxLayout->addWidget(_useOpenGL);

	QWidget *systemTab = new QWidget();
	QVBoxLayout *systemTabLayout = new QVBoxLayout();
	systemTabLayout->addLayout(formLayout);
	systemTabLayout->addLayout(checkboxLayout);
	systemTabLayout->addStretch();
	systemTab->setLayout(systemTabLayout);

	QTabWidget *systemPage = new QTabWidget();
	systemPage->addTab(systemTab, tr("System"));

	return systemPage;
}

OptionsDialog::OptionsDialog(Options *options, QWidget *parent)
  : QDialog(parent), _options(options)
{
	QStackedWidget *pages = new QStackedWidget();
	pages->addWidget(createAppearancePage());
	pages->addWidget(createMapPage());
	pages->addWidget(createDataPage());
	pages->addWidget(createPOIPage());
	pages->addWidget(createExportPage());
	pages->addWidget(createSystemPage());

	QListWidget *menu = new QListWidget();
	menu->setIconSize(QSize(MENU_ICON_SIZE, MENU_ICON_SIZE));
	new QListWidgetItem(QIcon(APPEARANCE_ICON), tr("Appearance"),
	  menu);
	new QListWidgetItem(QIcon(MAPS_ICON), tr("Maps"), menu);
	new QListWidgetItem(QIcon(DATA_ICON), tr("Data"), menu);
	new QListWidgetItem(QIcon(POI_ICON), tr("POI"), menu);
	new QListWidgetItem(QIcon(PRINT_EXPORT_ICON), tr("Print & Export"),
	  menu);
	new QListWidgetItem(QIcon(SYSTEM_ICON), tr("System"), menu);

	QHBoxLayout *contentLayout = new QHBoxLayout();
	contentLayout->addWidget(menu);
	contentLayout->addWidget(pages);

	menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	menu->setMaximumWidth(menu->sizeHintForColumn(0) + 2 * menu->frameWidth()
	  + MENU_MARGIN);
	pages->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	pages->setMinimumWidth(2 * menu->size().width());

	connect(menu, SIGNAL(currentRowChanged(int)), pages,
	  SLOT(setCurrentIndex(int)));
	menu->item(0)->setSelected(true);


	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
	  | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(contentLayout);
	layout->addWidget(buttonBox);
	setLayout(layout);

	setWindowTitle(tr("Options"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void OptionsDialog::accept()
{
	_options->palette.setColor(_baseColor->color());
	_options->palette.setShift(_colorOffset->value());
	_options->mapOpacity = _mapOpacity->value();
	_options->backgroundColor = _backgroundColor->color();
	_options->trackWidth = _trackWidth->value();
	_options->trackStyle = (Qt::PenStyle) _trackStyle->itemData(
	  _trackStyle->currentIndex()).toInt();
	_options->routeWidth = _routeWidth->value();
	_options->routeStyle = (Qt::PenStyle) _routeStyle->itemData(
	  _routeStyle->currentIndex()).toInt();
	_options->pathAntiAliasing = _pathAA->isChecked();
	_options->waypointSize = _waypointSize->value();
	_options->waypointColor = _waypointColor->color();
	_options->poiSize = _poiSize->value();
	_options->poiColor = _poiColor->color();
	_options->graphWidth = _graphWidth->value();
	_options->sliderColor = _sliderColor->color();
	_options->graphAntiAliasing = _graphAA->isChecked();

	_options->alwaysShowMap = _alwaysShowMap->isChecked();
#ifdef ENABLE_HIDPI
	_options->hidpiMap = _hidpi->isChecked();
#endif // ENABLE_HIDPI

	_options->elevationFilter = _elevationFilter->value();
	_options->speedFilter = _speedFilter->value();
	_options->heartRateFilter = _heartRateFilter->value();
	_options->cadenceFilter = _cadenceFilter->value();
	_options->powerFilter = _powerFilter->value();
	_options->outlierEliminate = _outlierEliminate->isChecked();
	qreal pauseSpeed = (_options->units == Imperial)
		? _pauseSpeed->value() / MS2MIH : (_options->units == Nautical)
		? _pauseSpeed->value() / MS2KN : _pauseSpeed->value() / MS2KMH;
	if (qAbs(pauseSpeed - _options->pauseSpeed) > 0.01)
		_options->pauseSpeed = pauseSpeed;
	_options->pauseInterval = _pauseInterval->value();
	_options->useReportedSpeed = _reported->isChecked();

	qreal poiRadius = (_options->units == Imperial)
		? _poiRadius->value() * MIINM : (_options->units == Nautical)
		? _poiRadius->value() * NMIINM : _poiRadius->value() * KMINM;
	if (qAbs(poiRadius - _options->poiRadius) > 0.01)
		_options->poiRadius = poiRadius;

	_options->useOpenGL = _useOpenGL->isChecked();
#ifdef ENABLE_HTTP2
	_options->enableHTTP2 = _enableHTTP2->isChecked();
#endif // ENABLE_HTTP2
	_options->pixmapCache = _pixmapCache->value();
	_options->connectionTimeout = _connectionTimeout->value();

	_options->hiresPrint = _hires->isChecked();
	_options->printName = _name->isChecked();
	_options->printDate = _date->isChecked();
	_options->printDistance = _distance->isChecked();
	_options->printTime = _time->isChecked();
	_options->printMovingTime = _movingTime->isChecked();
	_options->printItemCount = _itemCount->isChecked();
	_options->separateGraphPage = _separateGraphPage->isChecked();

	QDialog::accept();
}
