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
#include <QButtonGroup>
#include "map/pcs.h"
#include "icons.h"
#include "colorbox.h"
#include "stylecombobox.h"
#include "oddspinbox.h"
#include "percentslider.h"
#include "limitedcombobox.h"
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
#endif // Q_OS_MAC


void OptionsDialog::automaticPauseDetectionSet(bool set)
{
	_pauseInterval->setEnabled(!set);
	_pauseSpeed->setEnabled(!set);
}

QWidget *OptionsDialog::createMapPage()
{
	_projection = new LimitedComboBox(200);

	QList<KV<int, QString> > projections(GCS::list() + PCS::list());
	qSort(projections);

	for (int i = 0; i < projections.size(); i++) {
		QString text = QString::number(projections.at(i).key()) + " - "
		  + projections.at(i).value();
		_projection->addItem(text, QVariant(projections.at(i).key()));
	}
	_projection->setCurrentIndex(_projection->findData(_options->projection));

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

	QFormLayout *vectorLayout = new QFormLayout();
	vectorLayout->addRow(tr("Projection:"), _projection);

	QWidget *vectorMapsTab = new QWidget();
	QVBoxLayout *vectorMapsTabLayout = new QVBoxLayout();
	vectorMapsTabLayout->addLayout(vectorLayout);
	vectorMapsTabLayout->addStretch();
	vectorMapsTab->setLayout(vectorMapsTabLayout);

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
	mapPage->addTab(vectorMapsTab, tr("Vector maps"));
#ifdef ENABLE_HIDPI
	mapPage->addTab(hidpiTab, tr("HiDPI display mode"));
#endif // ENABLE_HIDPI

	return mapPage;
}

QWidget *OptionsDialog::createAppearancePage()
{
	// Tracks
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

	// Routes
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

	// Areas
	_areaWidth = new QSpinBox();
	_areaWidth->setValue(_options->areaWidth);
	_areaStyle = new StyleComboBox();
	_areaStyle->setValue(_options->areaStyle);
	_areaOpacity = new PercentSlider();
	_areaOpacity->setValue(_options->areaOpacity);
	QFormLayout *areaLayout = new QFormLayout();
#ifdef Q_OS_MAC
	areaLayout->addRow(tr("Area border width:"), _areaWidth);
	areaLayout->addRow(tr("Area border style:"), _areaStyle);
	areaLayout->addRow(tr("Area fill opacity:"), _areaOpacity);
#else // Q_OS_MAC
	areaLayout->addRow(tr("Width:"), _areaWidth);
	areaLayout->addRow(tr("Style:"), _areaStyle);
	areaLayout->addRow(tr("Fill opacity:"), _areaOpacity);
	QGroupBox *areaBox = new QGroupBox(tr("Areas"));
	areaBox->setLayout(areaLayout);
#endif // Q_OS_MAC

	// Palette & antialiasing
	_baseColor = new ColorBox();
	_baseColor->setColor(_options->palette.color());
	_colorOffset = new PercentSlider();
	_colorOffset->setValue(_options->palette.shift() * 100);
	QFormLayout *paletteLayout = new QFormLayout();
	paletteLayout->addRow(tr("Base color:"), _baseColor);
	paletteLayout->addRow(tr("Palette shift:"), _colorOffset);

	_pathAA = new QCheckBox(tr("Use anti-aliasing"));
	_pathAA->setChecked(_options->pathAntiAliasing);
	QFormLayout *pathAALayout = new QFormLayout();
	pathAALayout->addWidget(_pathAA);

	QWidget *pathTab = new QWidget();
	QVBoxLayout *pathTabLayout = new QVBoxLayout();
#ifdef Q_OS_MAC
	pathTabLayout->addLayout(trackLayout);
	pathTabLayout->addWidget(line());
	pathTabLayout->addLayout(routeLayout);
	pathTabLayout->addWidget(line());
	pathTabLayout->addLayout(areaLayout);
	pathTabLayout->addWidget(line());
#else // Q_OS_MAC
	pathTabLayout->addWidget(trackBox);
	pathTabLayout->addWidget(routeBox);
	pathTabLayout->addWidget(areaBox);
#endif // Q_OS_MAC
	pathTabLayout->addLayout(paletteLayout);
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

	QWidget *filterTab = new QWidget();
	QVBoxLayout *filterTabLayout = new QVBoxLayout();
#ifdef Q_OS_MAC
	filterTabLayout->addWidget(new QLabel(tr("Smoothing:")));
	filterTabLayout->addLayout(smoothLayout);
	filterTabLayout->addWidget(line());
#else // Q_OS_MAC
	filterTabLayout->addWidget(smoothBox);
#endif // Q_OS_MAC
	filterTabLayout->addLayout(outlierLayout);
	filterTabLayout->addStretch();
	filterTab->setLayout(filterTabLayout);


	_automaticPause = new QRadioButton(tr("Automatic"));
	_manualPause = new QRadioButton(tr("Custom"));
	if (_options->automaticPause)
		_automaticPause->setChecked(true);
	else
		_manualPause->setChecked(true);

	_pauseSpeed = new QDoubleSpinBox();
	_pauseSpeed->setDecimals(1);
	_pauseSpeed->setSingleStep(0.1);
	_pauseSpeed->setMinimum(0.1);
	_pauseSpeed->setEnabled(_manualPause->isChecked());
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
	_pauseInterval->setEnabled(_manualPause->isChecked());

	connect(_automaticPause, SIGNAL(toggled(bool)), this,
	  SLOT(automaticPauseDetectionSet(bool)));

	QHBoxLayout *pauseTypeLayout = new QHBoxLayout();
#ifdef Q_OS_MAC
	pauseTypeLayout->addStretch();
#endif
	pauseTypeLayout->addWidget(_automaticPause);
	pauseTypeLayout->addWidget(_manualPause);
	pauseTypeLayout->addStretch();

	QFormLayout *pauseValuesLayout = new QFormLayout();
	pauseValuesLayout->addRow(tr("Minimal speed:"), _pauseSpeed);
	pauseValuesLayout->addRow(tr("Minimal duration:"), _pauseInterval);

	QVBoxLayout *pauseLayout = new QVBoxLayout();
	pauseLayout->addLayout(pauseTypeLayout);
	pauseLayout->addLayout(pauseValuesLayout);

	QWidget *pauseTab = new QWidget();
	pauseTab->setLayout(pauseLayout);


	_computedSpeed = new QRadioButton(tr("Computed from distance/time"));
	_reportedSpeed = new QRadioButton(tr("Recorded by device"));
	if (_options->useReportedSpeed)
		_reportedSpeed->setChecked(true);
	else
		_computedSpeed->setChecked(true);
	_showSecondarySpeed = new QCheckBox(tr("Show secondary speed"));
	_showSecondarySpeed->setChecked(_options->showSecondarySpeed);

	_dataGPSElevation = new QRadioButton(tr("GPS data"));
	_dataDEMElevation = new QRadioButton(tr("DEM data"));
	if (_options->dataUseDEM)
		_dataDEMElevation->setChecked(true);
	else
		_dataGPSElevation->setChecked(true);
	_showSecondaryElevation = new QCheckBox(tr("Show secondary elevation"));
	_showSecondaryElevation->setChecked(_options->showSecondaryElevation);

#ifdef ENABLE_TIMEZONES
	_utcZone = new QRadioButton(tr("UTC"));
	_systemZone = new QRadioButton(tr("System"));
	_customZone = new QRadioButton(tr("Custom"));
	if (_options->timeZone.type() == TimeZoneInfo::UTC)
		_utcZone->setChecked(true);
	else if (_options->timeZone.type() == TimeZoneInfo::System)
		_systemZone->setChecked(true);
	else
		_customZone->setChecked(true);
	_timeZone = new QComboBox();
	_timeZone->setEnabled(_customZone->isChecked());
	QList<QByteArray> zones = QTimeZone::availableTimeZoneIds();
	for (int i = 0; i < zones.size(); i++)
		_timeZone->addItem(zones.at(i));
	_timeZone->setCurrentText(_options->timeZone.customZone().id());
	connect(_customZone, SIGNAL(toggled(bool)), _timeZone,
	  SLOT(setEnabled(bool)));
	QHBoxLayout *customZoneLayout = new QHBoxLayout();
	customZoneLayout->addSpacing(20);
	customZoneLayout->addWidget(_timeZone);
#endif // ENABLE_TIMEZONES

	QWidget *sourceTab = new QWidget();
	QVBoxLayout *sourceTabLayout = new QVBoxLayout();

#ifdef Q_OS_MAC
	QButtonGroup *speedGroup = new QButtonGroup(this);
	speedGroup->addButton(_computedSpeed);
	speedGroup->addButton(_reportedSpeed);
	QVBoxLayout *speedOptions = new QVBoxLayout();
	speedOptions->addWidget(_computedSpeed);
	speedOptions->addWidget(_reportedSpeed);
	speedOptions->addWidget(_showSecondarySpeed);

	QButtonGroup *elevationGroup = new QButtonGroup(this);
	elevationGroup->addButton(_dataGPSElevation);
	elevationGroup->addButton(_dataDEMElevation);
	QVBoxLayout *elevationOptions = new QVBoxLayout();
	elevationOptions->addWidget(_dataGPSElevation);
	elevationOptions->addWidget(_dataDEMElevation);
	elevationOptions->addWidget(_showSecondaryElevation);

#ifdef ENABLE_TIMEZONES
	QButtonGroup *timeZoneGroup = new QButtonGroup(this);
	timeZoneGroup->addButton(_utcZone);
	timeZoneGroup->addButton(_systemZone);
	timeZoneGroup->addButton(_customZone);
	QVBoxLayout *zoneOptions = new QVBoxLayout();
	zoneOptions->addWidget(_utcZone);
	zoneOptions->addWidget(_systemZone);
	zoneOptions->addWidget(_customZone);
	zoneOptions->addItem(customZoneLayout);
#endif // ENABLE_TIMEZONES

	QFormLayout *formLayout = new QFormLayout();
	formLayout->addRow(tr("Speed:"), speedOptions);
	formLayout->addRow(tr("Elevation:"), elevationOptions);
#ifdef ENABLE_TIMEZONES
	formLayout->addRow(tr("Time zone:"), zoneOptions);
#endif // ENABLE_TIMEZONES

	sourceTabLayout->addLayout(formLayout);
#else // Q_OS_MAC
	QFormLayout *speedLayout = new QFormLayout();
	QFormLayout *elevationLayout = new QFormLayout();
#ifdef ENABLE_TIMEZONES
	QFormLayout *timeZoneLayout = new QFormLayout();
#endif // ENABLE_TIMEZONES

	speedLayout->addWidget(_computedSpeed);
	speedLayout->addWidget(_reportedSpeed);
	speedLayout->addWidget(_showSecondarySpeed);

	QGroupBox *speedBox = new QGroupBox(tr("Speed"));
	speedBox->setLayout(speedLayout);

	elevationLayout->addWidget(_dataGPSElevation);
	elevationLayout->addWidget(_dataDEMElevation);
	elevationLayout->addWidget(_showSecondaryElevation);

	QGroupBox *elevationBox = new QGroupBox(tr("Elevation"));
	elevationBox->setLayout(elevationLayout);

#ifdef ENABLE_TIMEZONES
	timeZoneLayout->addWidget(_utcZone);
	timeZoneLayout->addWidget(_systemZone);
	timeZoneLayout->addWidget(_customZone);
	timeZoneLayout->addItem(customZoneLayout);

	QGroupBox *timeZoneBox = new QGroupBox(tr("Time zone"));
	timeZoneBox->setLayout(timeZoneLayout);
#endif // ENABLE_TIMEZONES

	sourceTabLayout->addWidget(speedBox);
	sourceTabLayout->addWidget(elevationBox);
#ifdef ENABLE_TIMEZONES
	sourceTabLayout->addWidget(timeZoneBox);
#endif // ENABLE_TIMEZONES
#endif // Q_OS_MAC
	sourceTabLayout->addStretch();
	sourceTab->setLayout(sourceTabLayout);


	QTabWidget *filterPage = new QTabWidget();
	filterPage->addTab(filterTab, tr("Filtering"));
	filterPage->addTab(sourceTab, tr("Sources"));
	filterPage->addTab(pauseTab, tr("Pause detection"));

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
	poiLayout->addRow(tr("Radius:"), _poiRadius);

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
	_options->palette.setShift(_colorOffset->value() / 100.0);
	_options->mapOpacity = _mapOpacity->value();
	_options->backgroundColor = _backgroundColor->color();
	_options->trackWidth = _trackWidth->value();
	_options->trackStyle = (Qt::PenStyle) _trackStyle->itemData(
	  _trackStyle->currentIndex()).toInt();
	_options->routeWidth = _routeWidth->value();
	_options->routeStyle = (Qt::PenStyle) _routeStyle->itemData(
	  _routeStyle->currentIndex()).toInt();
	_options->pathAntiAliasing = _pathAA->isChecked();
	_options->areaWidth = _areaWidth->value();
	_options->areaStyle = (Qt::PenStyle) _areaStyle->itemData(
	  _areaStyle->currentIndex()).toInt();
	_options->areaOpacity = _areaOpacity->value();
	_options->waypointSize = _waypointSize->value();
	_options->waypointColor = _waypointColor->color();
	_options->poiSize = _poiSize->value();
	_options->poiColor = _poiColor->color();
	_options->graphWidth = _graphWidth->value();
	_options->sliderColor = _sliderColor->color();
	_options->graphAntiAliasing = _graphAA->isChecked();

	_options->projection = _projection->itemData(_projection->currentIndex())
	  .toInt();
#ifdef ENABLE_HIDPI
	_options->hidpiMap = _hidpi->isChecked();
#endif // ENABLE_HIDPI

	_options->elevationFilter = _elevationFilter->value();
	_options->speedFilter = _speedFilter->value();
	_options->heartRateFilter = _heartRateFilter->value();
	_options->cadenceFilter = _cadenceFilter->value();
	_options->powerFilter = _powerFilter->value();
	_options->outlierEliminate = _outlierEliminate->isChecked();
	_options->automaticPause = _automaticPause->isChecked();
	qreal pauseSpeed = (_options->units == Imperial)
		? _pauseSpeed->value() / MS2MIH : (_options->units == Nautical)
		? _pauseSpeed->value() / MS2KN : _pauseSpeed->value() / MS2KMH;
	if (qAbs(pauseSpeed - _options->pauseSpeed) > 0.01)
		_options->pauseSpeed = pauseSpeed;
	_options->pauseInterval = _pauseInterval->value();
	_options->useReportedSpeed = _reportedSpeed->isChecked();
	_options->dataUseDEM = _dataDEMElevation->isChecked();
	_options->showSecondaryElevation = _showSecondaryElevation->isChecked();
	_options->showSecondarySpeed = _showSecondarySpeed->isChecked();
#ifdef ENABLE_TIMEZONES
	_options->timeZone.setType(_utcZone->isChecked()
	  ? TimeZoneInfo::UTC : _systemZone->isChecked()
	  ? TimeZoneInfo::System : TimeZoneInfo::Custom);
	_options->timeZone.setCustomZone(QTimeZone(_timeZone->currentText()
	  .toLatin1()));
#endif // ENABLE_TIMEZONES

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
