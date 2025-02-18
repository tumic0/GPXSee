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
#include <QGeoPositionInfoSource>
#include "map/pcs.h"
#include "icons.h"
#include "infolabel.h"
#include "colorbox.h"
#include "stylecombobox.h"
#include "oddspinbox.h"
#include "percentslider.h"
#include "projectioncombobox.h"
#include "dirselectwidget.h"
#include "authenticationwidget.h"
#include "pluginparameters.h"
#include "optionsdialog.h"

#ifdef Q_OS_ANDROID
#define MENU_MARGIN 0
#else // Q_OS_ANDROID
#define MENU_MARGIN 20
#endif // Q_OS_ANDROID
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

typedef QList<KV<int, QString> > ProjectionList;

void OptionsDialog::automaticPauseDetectionSet(bool set)
{
	_pauseInterval->setEnabled(!set);
	_pauseSpeed->setEnabled(!set);
}

void OptionsDialog::pauseDetectionSet(bool set)
{
	_automaticPause->setEnabled(set);
	_manualPause->setEnabled(set);
	_pauseInterval->setEnabled(set && _manualPause->isChecked());
	_pauseSpeed->setEnabled(set && _manualPause->isChecked());
}

QWidget *OptionsDialog::createMapPage()
{
	ProjectionList outputProjections(GCS::WGS84List() + Conversion::list());
	ProjectionList inputProjections(GCS::list() + PCS::list());
	std::sort(outputProjections.begin(), outputProjections.end());
	std::sort(inputProjections.begin(), inputProjections.end());

	_outputProjection = new ProjectionComboBox(outputProjections);
	_outputProjection->setCurrentIndex(_outputProjection->findData(
	  _options.outputProjection));
	_inputProjection = new ProjectionComboBox(inputProjections);
	_inputProjection->setCurrentIndex(_inputProjection->findData(
	  _options.inputProjection));

	InfoLabel *inInfo = new InfoLabel(tr("Select the proper coordinate "
	  "reference system (CRS) of maps without a CRS definition "
	  "(JNX, KMZ and World file maps)."));
	InfoLabel *outInfo = new InfoLabel(tr("Select the desired projection of "
	  "vector maps (IMG, Mapsforge and ENC maps). The projection must be valid "
	  "for the whole map area."));

	_hidpi = new QRadioButton(tr("High-Resolution"));
	_lodpi = new QRadioButton(tr("Standard"));
	if (_options.hidpiMap)
		_hidpi->setChecked(true);
	else
		_lodpi->setChecked(true);
	InfoLabel *lhi = new InfoLabel(tr("Non-HiDPI maps are loaded as HiDPI maps. "
	  "The map is sharp but map objects are small/hard to read."));
	InfoLabel *llo = new InfoLabel(tr("Non-HiDPI maps are loaded such as they are. "
	  "Map objects have the expected size but the map is blurry."));

	QVBoxLayout *inLayout = new QVBoxLayout();
	inLayout->addWidget(_inputProjection);
	inLayout->addWidget(inInfo);
	QVBoxLayout *outLayout = new QVBoxLayout();
	outLayout->addWidget(_outputProjection);
	outLayout->addWidget(outInfo);
#ifndef Q_OS_MAC
	QGroupBox *inBox = new QGroupBox(tr("Input"));
	inBox->setLayout(inLayout);
	QGroupBox *outBox = new QGroupBox(tr("Output"));
	outBox->setLayout(outLayout);
#endif // Q_OS_MAC

	QWidget *projectionTab = new QWidget();
	QVBoxLayout *projectionTabLayout = new QVBoxLayout();
#ifdef Q_OS_MAC
	projectionTabLayout->addWidget(new QLabel(tr("Input:")));
	projectionTabLayout->addLayout(inLayout);
	projectionTabLayout->addWidget(line());
	projectionTabLayout->addWidget(new QLabel(tr("Output:")));
	projectionTabLayout->addLayout(outLayout);
#else // Q_OS_MAC
	projectionTabLayout->addWidget(inBox);
	projectionTabLayout->addWidget(outBox);
#endif // Q_OS_MAC
	projectionTabLayout->addStretch();
	projectionTab->setLayout(projectionTabLayout);

	QVBoxLayout *hidpiTabLayout = new QVBoxLayout();
	hidpiTabLayout->addWidget(_lodpi);
	hidpiTabLayout->addWidget(llo);
	hidpiTabLayout->addSpacing(10);
	hidpiTabLayout->addWidget(_hidpi);
	hidpiTabLayout->addWidget(lhi);
	hidpiTabLayout->addStretch();

	QWidget *hidpiTab = new QWidget();
	hidpiTab->setLayout(hidpiTabLayout);

	QTabWidget *mapPage = new QTabWidget();
	mapPage->addTab(projectionTab, tr("Projection"));
	mapPage->addTab(hidpiTab, tr("HiDPI display mode"));

	return mapPage;
}

QWidget *OptionsDialog::createAppearancePage()
{
	// Tracks
	_trackWidth = new QSpinBox();
	_trackWidth->setMinimum(1);
	_trackWidth->setSuffix(UNIT_SPACE + tr("px"));
	_trackWidth->setValue(_options.trackWidth);
	_trackStyle = new StyleComboBox();
	_trackStyle->setValue(_options.trackStyle);
	// Routes
	_routeWidth = new QSpinBox();
	_routeWidth->setMinimum(1);
	_routeWidth->setSuffix(UNIT_SPACE + tr("px"));
	_routeWidth->setValue(_options.routeWidth);
	_routeStyle = new StyleComboBox();
	_routeStyle->setValue(_options.routeStyle);
	// Areas
	_areaWidth = new QSpinBox();
	_areaWidth->setSuffix(UNIT_SPACE + tr("px"));
	_areaWidth->setValue(_options.areaWidth);
	_areaStyle = new StyleComboBox();
	_areaStyle->setValue(_options.areaStyle);
	_areaOpacity = new PercentSlider();
	_areaOpacity->setValue(_options.areaOpacity);
	// Palette & antialiasing
	_baseColor = new ColorBox();
	_baseColor->setColor(_options.palette.color());
	_colorOffset = new PercentSlider();
	_colorOffset->setValue(_options.palette.shift() * 100);
	_pathAA = new QCheckBox(tr("Use anti-aliasing"));
	_pathAA->setChecked(_options.pathAntiAliasing);

#ifdef Q_OS_MAC
	QWidget *pathTab = new QWidget();
	QFormLayout *pathTabLayout = new QFormLayout();
	pathTabLayout->addRow(tr("Track width:"), _trackWidth);
	pathTabLayout->addRow(tr("Track style:"), _trackStyle);
	pathTabLayout->addRow(line());
	pathTabLayout->addRow(tr("Route width:"), _routeWidth);
	pathTabLayout->addRow(tr("Route style:"), _routeStyle);
	pathTabLayout->addRow(line());
	pathTabLayout->addRow(tr("Area border width:"), _areaWidth);
	pathTabLayout->addRow(tr("Area border style:"), _areaStyle);
	pathTabLayout->addRow(tr("Area fill opacity:"), _areaOpacity);
	pathTabLayout->addRow(line());
	pathTabLayout->addRow(tr("Base color:"), _baseColor);
	pathTabLayout->addRow(tr("Palette shift:"), _colorOffset);
	pathTabLayout->addRow(line());
	pathTabLayout->addWidget(_pathAA);
	pathTab->setLayout(pathTabLayout);
#else // Q_OS_MAC
	QFormLayout *trackLayout = new QFormLayout();
	trackLayout->addRow(tr("Width:"), _trackWidth);
	trackLayout->addRow(tr("Style:"), _trackStyle);
	QGroupBox *trackBox = new QGroupBox(tr("Tracks"));
	trackBox->setLayout(trackLayout);
	QFormLayout *routeLayout = new QFormLayout();
	routeLayout->addRow(tr("Width:"), _routeWidth);
	routeLayout->addRow(tr("Style:"), _routeStyle);
	QGroupBox *routeBox = new QGroupBox(tr("Routes"));
	routeBox->setLayout(routeLayout);
	QFormLayout *areaLayout = new QFormLayout();
	areaLayout->addRow(tr("Width:"), _areaWidth);
	areaLayout->addRow(tr("Style:"), _areaStyle);
	areaLayout->addRow(tr("Opacity:"), _areaOpacity);
	QGroupBox *areaBox = new QGroupBox(tr("Areas"));
	areaBox->setLayout(areaLayout);
	QFormLayout *paletteLayout = new QFormLayout();
	paletteLayout->addRow(tr("Base color:"), _baseColor);
	paletteLayout->addRow(tr("Palette shift:"), _colorOffset);
	QFormLayout *pathAALayout = new QFormLayout();
	pathAALayout->addWidget(_pathAA);
	QWidget *pathTab = new QWidget();
	QVBoxLayout *pathTabLayout = new QVBoxLayout();
	pathTabLayout->addWidget(trackBox);
	pathTabLayout->addWidget(routeBox);
	pathTabLayout->addWidget(areaBox);
	pathTabLayout->addLayout(paletteLayout);
	pathTabLayout->addLayout(pathAALayout);
	pathTabLayout->addStretch();
	pathTab->setLayout(pathTabLayout);
#endif // Q_OS_MAC

	// Waypoints
	_waypointSize = new QSpinBox();
	_waypointSize->setMinimum(1);
	_waypointSize->setSuffix(UNIT_SPACE + tr("px"));
	_waypointSize->setValue(_options.waypointSize);
	_waypointColor = new ColorBox();
	_waypointColor->setColor(_options.waypointColor);
	// POI
	_poiSize = new QSpinBox();
	_poiSize->setMinimum(1);
	_poiSize->setSuffix(UNIT_SPACE + tr("px"));
	_poiSize->setValue(_options.poiSize);
	_poiColor = new ColorBox();
	_poiColor->setColor(_options.poiColor);

#ifdef Q_OS_MAC
	QWidget *pointTab = new QWidget();
	QFormLayout *pointTabLayout = new QFormLayout();
	pointTabLayout->addRow(tr("Waypoint color:"), _waypointColor);
	pointTabLayout->addRow(tr("Waypoint size:"), _waypointSize);
	pointTabLayout->addRow(line());
	pointTabLayout->addRow(tr("POI color:"), _poiColor);
	pointTabLayout->addRow(tr("POI size:"), _poiSize);
	pointTab->setLayout(pointTabLayout);
#else // Q_OS_MAC
	QFormLayout *waypointLayout = new QFormLayout();
	waypointLayout->addRow(tr("Color:"), _waypointColor);
	waypointLayout->addRow(tr("Size:"), _waypointSize);
	QGroupBox *waypointBox = new QGroupBox(tr("Waypoints"));
	waypointBox->setLayout(waypointLayout);
	QFormLayout *poiLayout = new QFormLayout();
	poiLayout->addRow(tr("Color:"), _poiColor);
	poiLayout->addRow(tr("Size:"), _poiSize);
	QGroupBox *poiBox = new QGroupBox(tr("POIs"));
	poiBox->setLayout(poiLayout);
	QWidget *pointTab = new QWidget();
	QVBoxLayout *pointTabLayout = new QVBoxLayout();
	pointTabLayout->addWidget(waypointBox);
	pointTabLayout->addWidget(poiBox);
	pointTabLayout->addStretch();
	pointTab->setLayout(pointTabLayout);
#endif // Q_OS_MAC

	// Graphs
	_sliderColor = new ColorBox();
	_sliderColor->setColor(_options.sliderColor);
	_graphWidth = new QSpinBox();
	_graphWidth->setMinimum(1);
	_graphWidth->setSuffix(UNIT_SPACE + tr("px"));
	_graphWidth->setValue(_options.graphWidth);
	_graphAA = new QCheckBox(tr("Use anti-aliasing"));
	_graphAA->setChecked(_options.graphAntiAliasing);

#ifdef Q_OS_MAC
	QWidget *graphTab = new QWidget();
	QFormLayout *graphTabLayout = new QFormLayout();
	graphTabLayout->addRow(tr("Line width:"), _graphWidth);
	graphTabLayout->addRow(tr("Slider color:"), _sliderColor);
	graphTabLayout->addWidget(_graphAA);
	graphTab->setLayout(graphTabLayout);
#else // Q_OS_MAC
	QFormLayout *graphLayout = new QFormLayout();
	graphLayout->addRow(tr("Line width:"), _graphWidth);
	graphLayout->addRow(tr("Slider color:"), _sliderColor);
	QFormLayout *graphAALayout = new QFormLayout();
	graphAALayout->addWidget(_graphAA);
	QWidget *graphTab = new QWidget();
	QVBoxLayout *graphTabLayout = new QVBoxLayout();
	graphTabLayout->addLayout(graphLayout);
	graphTabLayout->addLayout(graphAALayout);
	graphTabLayout->addStretch();
	graphTab->setLayout(graphTabLayout);
#endif // Q_OS_MAC

	// Map
	_mapOpacity = new PercentSlider();
	_mapOpacity->setValue(_options.mapOpacity);
	_backgroundColor = new ColorBox();
	_backgroundColor->setColor(_options.backgroundColor);
	_backgroundColor->enableAlphaChannel(false);
	_crosshairColor = new ColorBox();
	_crosshairColor->setColor(_options.crosshairColor);
	_infoColor = new ColorBox();
	_infoColor->setColor(_options.infoColor);
	_infoBackground = new QCheckBox(tr("Info background"));
	_infoBackground->setChecked(_options.infoBackground);

	QFormLayout *mapLayout = new QFormLayout();
	mapLayout->addRow(tr("Background color:"), _backgroundColor);
	mapLayout->addRow(tr("Opacity:"), _mapOpacity);
	mapLayout->addRow(tr("Crosshair color:"), _crosshairColor);
	mapLayout->addRow(tr("Info color:"), _infoColor);
	mapLayout->addWidget(_infoBackground);

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
	_elevationFilter->setValue(_options.elevationFilter);
	_elevationFilter->setToolTip(filterToolTip);
	_speedFilter = new OddSpinBox();
	_speedFilter->setValue(_options.speedFilter);
	_speedFilter->setToolTip(filterToolTip);
	_heartRateFilter = new OddSpinBox();
	_heartRateFilter->setValue(_options.heartRateFilter);
	_heartRateFilter->setToolTip(filterToolTip);
	_cadenceFilter = new OddSpinBox();
	_cadenceFilter->setValue(_options.cadenceFilter);
	_cadenceFilter->setToolTip(filterToolTip);
	_powerFilter = new OddSpinBox();
	_powerFilter->setValue(_options.powerFilter);
	_powerFilter->setToolTip(filterToolTip);

	_outlierEliminate = new QCheckBox(tr("Eliminate GPS outliers"));
	_outlierEliminate->setChecked(_options.outlierEliminate);

#ifdef Q_OS_MAC
	QWidget *filterTab = new QWidget();
	QFormLayout *filterTabLayout = new QFormLayout();
	filterTabLayout->addWidget(new QLabel(tr("Smoothing")));
	filterTabLayout->addRow(tr("Elevation:"), _elevationFilter);
	filterTabLayout->addRow(tr("Speed:"), _speedFilter);
	filterTabLayout->addRow(tr("Heart rate:"), _heartRateFilter);
	filterTabLayout->addRow(tr("Cadence:"), _cadenceFilter);
	filterTabLayout->addRow(tr("Power:"), _powerFilter);
	filterTabLayout->addWidget(new QWidget());
	filterTabLayout->addWidget(_outlierEliminate);
	filterTab->setLayout(filterTabLayout);
#else // Q_OS_MAC
	QFormLayout *smoothLayout = new QFormLayout();
	smoothLayout->addRow(tr("Elevation:"), _elevationFilter);
	smoothLayout->addRow(tr("Speed:"), _speedFilter);
	smoothLayout->addRow(tr("Heart rate:"), _heartRateFilter);
	smoothLayout->addRow(tr("Cadence:"), _cadenceFilter);
	smoothLayout->addRow(tr("Power:"), _powerFilter);
	QWidget *filterTab = new QWidget();
	QVBoxLayout *filterTabLayout = new QVBoxLayout();
	QGroupBox *smoothBox = new QGroupBox(tr("Smoothing"));
	smoothBox->setLayout(smoothLayout);
	QFormLayout *outlierLayout = new QFormLayout();
	outlierLayout->addWidget(_outlierEliminate);
	filterTabLayout->addWidget(smoothBox);
	filterTabLayout->addLayout(outlierLayout);
	filterTabLayout->addStretch();
	filterTab->setLayout(filterTabLayout);
#endif // Q_OS_MAC

	_detectPauses = new QCheckBox(tr("Detect pauses"));
	_detectPauses->setChecked(_options.detectPauses);

	_automaticPause = new QRadioButton(tr("Automatic"));
	_manualPause = new QRadioButton(tr("Custom"));
	if (_options.automaticPause)
		_automaticPause->setChecked(true);
	else
		_manualPause->setChecked(true);

	_pauseSpeed = new QDoubleSpinBox();
	_pauseSpeed->setDecimals(1);
	_pauseSpeed->setSingleStep(0.1);
	_pauseSpeed->setMinimum(0.1);
	_pauseSpeed->setEnabled(_manualPause->isChecked());
	if (_units == Imperial) {
		_pauseSpeed->setValue(_options.pauseSpeed * MS2MIH);
		_pauseSpeed->setSuffix(UNIT_SPACE + tr("mi/h"));
	} else if (_units == Nautical) {
		_pauseSpeed->setValue(_options.pauseSpeed * MS2KN);
		_pauseSpeed->setSuffix(UNIT_SPACE + tr("kn"));
	} else {
		_pauseSpeed->setValue(_options.pauseSpeed * MS2KMH);
		_pauseSpeed->setSuffix(UNIT_SPACE + tr("km/h"));
	}
	_pauseInterval = new QSpinBox();
	_pauseInterval->setMinimum(1);
	_pauseInterval->setSuffix(UNIT_SPACE + tr("s"));
	_pauseInterval->setValue(_options.pauseInterval);
	_pauseInterval->setEnabled(_manualPause->isChecked());

	pauseDetectionSet(_options.detectPauses);

	connect(_detectPauses, &QCheckBox::toggled, this,
	  &OptionsDialog::pauseDetectionSet);
	connect(_automaticPause, &QRadioButton::toggled, this,
	  &OptionsDialog::automaticPauseDetectionSet);

	_computedSpeed = new QRadioButton(tr("Computed from distance/time"));
	_reportedSpeed = new QRadioButton(tr("Recorded by device"));
	if (_options.useReportedSpeed)
		_reportedSpeed->setChecked(true);
	else
		_computedSpeed->setChecked(true);
	_showSecondarySpeed = new QCheckBox(tr("Show secondary speed"));
	_showSecondarySpeed->setChecked(_options.showSecondarySpeed);

	_dataGPSElevation = new QRadioButton(tr("GPS data"));
	_dataDEMElevation = new QRadioButton(tr("DEM data"));
	if (_options.dataUseDEM)
		_dataDEMElevation->setChecked(true);
	else
		_dataGPSElevation->setChecked(true);
	_showSecondaryElevation = new QCheckBox(tr("Show secondary elevation"));
	_showSecondaryElevation->setChecked(_options.showSecondaryElevation);

	_utcZone = new QRadioButton(tr("UTC"));
	_systemZone = new QRadioButton(tr("System"));
	_customZone = new QRadioButton(tr("Custom"));
	if (_options.timeZone.type() == TimeZoneInfo::UTC)
		_utcZone->setChecked(true);
	else if (_options.timeZone.type() == TimeZoneInfo::System)
		_systemZone->setChecked(true);
	else
		_customZone->setChecked(true);
	_timeZone = new QComboBox();
	_timeZone->setEnabled(_customZone->isChecked());
	QList<QByteArray> zones = QTimeZone::availableTimeZoneIds();
	for (int i = 0; i < zones.size(); i++)
		_timeZone->addItem(zones.at(i));
	_timeZone->setCurrentText(_options.timeZone.customZone().id());
	connect(_customZone, &QRadioButton::toggled, _timeZone,
	  &QComboBox::setEnabled);
	QHBoxLayout *customZoneLayout = new QHBoxLayout();
	customZoneLayout->addSpacing(20);
	customZoneLayout->addWidget(_timeZone);

	_useSegments = new QCheckBox(tr("Use segments"));
	_useSegments->setChecked(_options.useSegments);

#ifdef Q_OS_MAC
	QWidget *sourceTab = new QWidget();
	QFormLayout *sourceTabLayout = new QFormLayout();
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
	QButtonGroup *timeZoneGroup = new QButtonGroup(this);
	timeZoneGroup->addButton(_utcZone);
	timeZoneGroup->addButton(_systemZone);
	timeZoneGroup->addButton(_customZone);
	QVBoxLayout *zoneOptions = new QVBoxLayout();
	zoneOptions->addWidget(_utcZone);
	zoneOptions->addWidget(_systemZone);
	zoneOptions->addWidget(_customZone);
	zoneOptions->addItem(customZoneLayout);
	sourceTabLayout->addRow(tr("Speed:"), speedOptions);
	sourceTabLayout->addRow(tr("Elevation:"), elevationOptions);
	sourceTabLayout->addRow(tr("Time zone:"), zoneOptions);
	sourceTabLayout->addRow(line());
	sourceTabLayout->addWidget(_useSegments);
	sourceTab->setLayout(sourceTabLayout);
#else // Q_OS_MAC
	QWidget *sourceTab = new QWidget();
	QVBoxLayout *sourceTabLayout = new QVBoxLayout();
	QFormLayout *speedLayout = new QFormLayout();
	QFormLayout *elevationLayout = new QFormLayout();
	QFormLayout *timeZoneLayout = new QFormLayout();
	QFormLayout *segmentsLayout = new QFormLayout();
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
	timeZoneLayout->addWidget(_utcZone);
	timeZoneLayout->addWidget(_systemZone);
	timeZoneLayout->addWidget(_customZone);
	timeZoneLayout->addItem(customZoneLayout);
	QGroupBox *timeZoneBox = new QGroupBox(tr("Time zone"));
	timeZoneBox->setLayout(timeZoneLayout);
	segmentsLayout->addWidget(_useSegments);
	sourceTabLayout->addWidget(speedBox);
	sourceTabLayout->addWidget(elevationBox);
	sourceTabLayout->addWidget(timeZoneBox);
	sourceTabLayout->addLayout(segmentsLayout);
	sourceTabLayout->addStretch();
	sourceTab->setLayout(sourceTabLayout);
#endif // Q_OS_MAC

	QVBoxLayout *pauseTypeLayout = new QVBoxLayout();
	pauseTypeLayout->addWidget(_automaticPause);
	pauseTypeLayout->addWidget(_manualPause);

	QFormLayout *pauseValuesLayout = new QFormLayout();
	pauseValuesLayout->addRow(tr("Detection:"), pauseTypeLayout);
	pauseValuesLayout->addRow(tr("Minimal speed:"), _pauseSpeed);
	pauseValuesLayout->addRow(tr("Minimal duration:"), _pauseInterval);

	QVBoxLayout *pauseLayout = new QVBoxLayout();
	pauseLayout->addWidget(_detectPauses);
	pauseLayout->addLayout(pauseValuesLayout);

	QWidget *pauseTab = new QWidget();
	pauseTab->setLayout(pauseLayout);


	QTabWidget *dataPage = new QTabWidget();
	dataPage->addTab(sourceTab, tr("Sources"));
	dataPage->addTab(filterTab, tr("Filtering"));
	dataPage->addTab(pauseTab, tr("Pause detection"));

	return dataPage;
}

QWidget *OptionsDialog::createPOIPage()
{
	_poiRadius = new QDoubleSpinBox();
	_poiRadius->setSingleStep(1);
	_poiRadius->setDecimals(1);
	if (_units == Imperial) {
		_poiRadius->setValue(_options.poiRadius / MIINM);
		_poiRadius->setSuffix(UNIT_SPACE + tr("mi"));
	} else if (_units == Nautical) {
		_poiRadius->setValue(_options.poiRadius / NMIINM);
		_poiRadius->setSuffix(UNIT_SPACE + tr("nmi"));
	} else {
		_poiRadius->setValue(_options.poiRadius / KMINM);
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

QWidget *OptionsDialog::createDEMPage()
{
	_demURL = new QLineEdit();
#ifndef Q_OS_ANDROID
	_demURL->setMinimumWidth(300);
#endif // Q_OS_ANDROID
	_demURL->setText(_options.demURL);
	_demAuth = new AuthenticationWidget();
	_demAuth->setUsername(_options.demUsername);
	_demAuth->setPassword(_options.demPassword);
	_demAuth->setEnabled(_options.demAuthorization);

	QCheckBox *useAuth = new QCheckBox(tr("Use HTTP authentication"));
	useAuth->setChecked(_demAuth->isEnabled());
	connect(useAuth, &QRadioButton::toggled, _demAuth,
	  &AuthenticationWidget::setEnabled);

	InfoLabel *info = new InfoLabel(
	  tr("Use $lat and $lon for NYY/SYY and EXXX/WXXX in the URL."));
	info->setMinimumWidth(_demURL->minimumWidth());

#ifdef Q_OS_MAC
	QFormLayout *sourceLayout = new QFormLayout();
	sourceLayout->addRow(tr("URL:"), _demURL);
	sourceLayout->addWidget(info);
	sourceLayout->addWidget(new QWidget());
	sourceLayout->addWidget(useAuth);
	sourceLayout->addWidget(_demAuth);
	sourceLayout->setAlignment(_demAuth, Qt::AlignLeft);
	QWidget *sourceTab = new QWidget();
	sourceTab->setLayout(sourceLayout);
#else // Q_OS_MAC
	QVBoxLayout *urlValueLayout = new QVBoxLayout();
	urlValueLayout->addWidget(_demURL);
	urlValueLayout->addWidget(info);
	QFormLayout *urlLayout = new QFormLayout();
	urlLayout->addRow(tr("URL:"), urlValueLayout);
	QVBoxLayout *sourceLayout = new QVBoxLayout();
	sourceLayout->addLayout(urlLayout);
	sourceLayout->addSpacing(10);
	sourceLayout->addWidget(useAuth);
	sourceLayout->addWidget(_demAuth);
	sourceLayout->addStretch();
	QWidget *sourceTab = new QWidget();
	sourceTab->setLayout(sourceLayout);
#endif // Q_OS_MAC

	_hillshadingAlpha = new PercentSlider();
	_hillshadingAlpha->setValue(qRound((_options.hillshadingAlpha / 255.0)
	  * 100));
	_hillshadingLightening = new PercentSlider();
	_hillshadingLightening->setValue(_options.hillshadingLightening * 100);
	_hillshadingBlur = new QSpinBox();
	_hillshadingBlur->setMaximum(10);
	_hillshadingBlur->setSuffix(UNIT_SPACE + tr("px"));
	_hillshadingBlur->setValue(_options.hillshadingBlur);
	_hillshadingAzimuth = new QSpinBox();
	_hillshadingAzimuth->setMaximum(360);
	_hillshadingAzimuth->setSuffix(UNIT_SPACE + QChar(0x00B0));
	_hillshadingAzimuth->setValue(_options.hillshadingAzimuth);
	_hillshadingAltitude = new QSpinBox();
	_hillshadingAltitude->setMaximum(90);
	_hillshadingAltitude->setSuffix(UNIT_SPACE + QChar(0x00B0));
	_hillshadingAltitude->setValue(_options.hillshadingAltitude);
	_hillshadingZFactor = new QDoubleSpinBox();
	_hillshadingZFactor->setDecimals(1);
	_hillshadingZFactor->setSingleStep(0.1);
	_hillshadingZFactor->setValue(_options.hillshadingZFactor);

	QFormLayout *hillshadingLayout = new QFormLayout();
	hillshadingLayout->addRow(tr("Opacity:"), _hillshadingAlpha);
	hillshadingLayout->addRow(tr("Lightening:"), _hillshadingLightening);
	hillshadingLayout->addRow(tr("Blur radius:"), _hillshadingBlur);
	hillshadingLayout->addItem(new QSpacerItem(10, 10));
	hillshadingLayout->addRow(tr("Azimuth:"), _hillshadingAzimuth);
	hillshadingLayout->addRow(tr("Altitude:"), _hillshadingAltitude);
	hillshadingLayout->addRow(tr("Z Factor:"), _hillshadingZFactor);
	QWidget *hillshadingTab = new QWidget();
	hillshadingTab->setLayout(hillshadingLayout);

	QTabWidget *demPage = new QTabWidget();
	demPage->addTab(sourceTab, tr("Source"));
	demPage->addTab(hillshadingTab, tr("Hillshading"));

	return demPage;
}

QWidget *OptionsDialog::createPositionPage()
{
	QStringList plugins(QGeoPositionInfoSource::availableSources());

	_positionPlugin = new QComboBox();
	_positionPlugin->addItems(plugins);
	_positionPlugin->setCurrentIndex(_positionPlugin->findText(_options.plugin));
	_pluginParameters = new PluginParameters(_positionPlugin->currentText(),
	  _options.pluginParams);
	connect(_positionPlugin, &QComboBox::currentTextChanged, _pluginParameters,
	  &PluginParameters::setPlugin);

	QFormLayout *pluginLayout = new QFormLayout();
	pluginLayout->addRow(tr("Plugin:"), _positionPlugin);

	QVBoxLayout *sourceLayout = new QVBoxLayout();
	sourceLayout->addLayout(pluginLayout);
	sourceLayout->addWidget(_pluginParameters);
	sourceLayout->addStretch();

	QWidget *sourceTab = new QWidget();
	sourceTab->setLayout(sourceLayout);

	QTabWidget *positionPage = new QTabWidget();
	positionPage->addTab(sourceTab, tr("Source"));

	return positionPage;
}

QWidget *OptionsDialog::createExportPage()
{
	_wysiwyg = new QRadioButton(tr("WYSIWYG"));
	_hires = new QRadioButton(tr("High-Resolution"));
	if (_options.hiresPrint)
		_hires->setChecked(true);
	else
		_wysiwyg->setChecked(true);
	InfoLabel *lw = new InfoLabel(tr("The printed area is approximately the "
	  "display area. The map zoom level does not change."));
	InfoLabel *lh = new InfoLabel(tr("The zoom level will be changed so that"
	  " the whole content (tracks/waypoints) fits to the printed area and"
	  " the map resolution is as close as possible to the print resolution."));

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
	_name->setChecked(_options.printName);
	_date = new QCheckBox(tr("Date"));
	_date->setChecked(_options.printDate);
	_distance = new QCheckBox(tr("Distance"));
	_distance->setChecked(_options.printDistance);
	_time = new QCheckBox(tr("Time"));
	_time->setChecked(_options.printTime);
	_movingTime = new QCheckBox(tr("Moving time"));
	_movingTime->setChecked(_options.printMovingTime);
	_itemCount = new QCheckBox(tr("Item count (>1)"));
	_itemCount->setChecked(_options.printItemCount);

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
	_separateGraphPage->setChecked(_options.separateGraphPage);

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
	_useOpenGL->setChecked(_options.useOpenGL);
	_enableHTTP2 = new QCheckBox(tr("Enable HTTP/2"));
	_enableHTTP2->setChecked(_options.enableHTTP2);

	_pixmapCache = new QSpinBox();
	_pixmapCache->setMinimum(64);
	_pixmapCache->setMaximum(4096);
	_pixmapCache->setSuffix(UNIT_SPACE + tr("MB"));
	_pixmapCache->setValue(_options.pixmapCache);

	_demCache = new QSpinBox();
	_demCache->setMinimum(64);
	_demCache->setMaximum(4096);
	_demCache->setSuffix(UNIT_SPACE + tr("MB"));
	_demCache->setValue(_options.demCache);

	_connectionTimeout = new QSpinBox();
	_connectionTimeout->setMinimum(30);
	_connectionTimeout->setMaximum(120);
	_connectionTimeout->setSuffix(UNIT_SPACE + tr("s"));
	_connectionTimeout->setValue(_options.connectionTimeout);

#ifdef Q_OS_MAC
	QWidget *systemTab = new QWidget();
	QFormLayout *systemTabLayout = new QFormLayout();
	systemTabLayout->addRow(tr("Image cache size:"), _pixmapCache);
	systemTabLayout->addRow(tr("DEM cache size:"), _demCache);
	systemTabLayout->addRow(tr("Connection timeout:"), _connectionTimeout);
	systemTabLayout->addWidget(_enableHTTP2);
	systemTabLayout->addWidget(_useOpenGL);
	systemTab->setLayout(systemTabLayout);
#else // Q_OS_MAC
	QFormLayout *formLayout = new QFormLayout();
	formLayout->addRow(tr("Image cache size:"), _pixmapCache);
	formLayout->addRow(tr("DEM cache size:"), _demCache);
	formLayout->addRow(tr("Connection timeout:"), _connectionTimeout);
	QFormLayout *checkboxLayout = new QFormLayout();
	checkboxLayout->addWidget(_enableHTTP2);
	checkboxLayout->addWidget(_useOpenGL);
	QWidget *systemTab = new QWidget();
	QVBoxLayout *systemTabLayout = new QVBoxLayout();
	systemTabLayout->addLayout(formLayout);
	systemTabLayout->addLayout(checkboxLayout);
	systemTabLayout->addStretch();
	systemTab->setLayout(systemTabLayout);
#endif // Q_OS_MAC

	_dataPath = new DirSelectWidget();
	_dataPath->setDir(_options.dataPath);
	_mapsPath = new DirSelectWidget();
	_mapsPath->setDir(_options.mapsPath);
	_poiPath = new DirSelectWidget();
	_poiPath->setDir(_options.poiPath);

	InfoLabel *info = new InfoLabel(tr("Select the initial paths of the file"
	  " open dialogues. Leave the field empty for the system default."));

	QFormLayout *pathsFormLayout = new QFormLayout();
	pathsFormLayout->addRow(tr("Data:"), _dataPath);
	pathsFormLayout->addRow(tr("Maps:"), _mapsPath);
	pathsFormLayout->addRow(tr("POI:"), _poiPath);

	QWidget *pathsTab = new QWidget();
	QVBoxLayout *pathsTabLayout = new QVBoxLayout();
	pathsTabLayout->addLayout(pathsFormLayout);
	pathsTabLayout->addWidget(info);
	pathsTabLayout->addStretch();
	pathsTab->setLayout(pathsTabLayout);

	QTabWidget *systemPage = new QTabWidget();
	systemPage->addTab(systemTab, tr("System"));
	systemPage->addTab(pathsTab, tr("Initial paths"));

	return systemPage;
}

OptionsDialog::OptionsDialog(Options &options, Units units, QWidget *parent)
  : QDialog(parent), _options(options), _units(units)
{
#ifdef Q_OS_ANDROID
	setWindowFlags(Qt::Window);
	setWindowState(Qt::WindowFullScreen);
#endif /* Q_OS_ANDROID */

	QStackedWidget *pages = new QStackedWidget();
	pages->addWidget(createAppearancePage());
	pages->addWidget(createMapPage());
	pages->addWidget(createDataPage());
	pages->addWidget(createPOIPage());
	pages->addWidget(createDEMPage());
	pages->addWidget(createPositionPage());
	pages->addWidget(createExportPage());
	pages->addWidget(createSystemPage());

	QListWidget *menu = new QListWidget();
	menu->setIconSize(QSize(MENU_ICON_SIZE, MENU_ICON_SIZE));
#ifdef Q_OS_ANDROID
	new QListWidgetItem(QIcon(APPEARANCE_ICON), QString(), menu);
	new QListWidgetItem(QIcon(MAPS_ICON), QString(), menu);
	new QListWidgetItem(QIcon(DATA_ICON), QString(), menu);
	new QListWidgetItem(QIcon(POI_ICON), QString(), menu);
	new QListWidgetItem(QIcon(DEM_ICON), QString(), menu);
	new QListWidgetItem(QIcon(POSITION_ICON), QString(), menu);
	new QListWidgetItem(QIcon(PRINT_EXPORT_ICON), QString(), menu);
	new QListWidgetItem(QIcon(SYSTEM_ICON), QString(), menu);
#else // Q_OS_ANDROID
	new QListWidgetItem(QIcon::fromTheme(APPEARANCE_NAME, QIcon(APPEARANCE_ICON)),
	  tr("Appearance"), menu);
	new QListWidgetItem(QIcon::fromTheme(MAPS_NAME, QIcon(MAPS_ICON)),
	  tr("Maps"), menu);
	new QListWidgetItem(QIcon::fromTheme(DATA_NAME, QIcon(DATA_ICON)),
	  tr("Data"), menu);
	new QListWidgetItem(QIcon::fromTheme(POI_NAME, QIcon(POI_ICON)), tr("POI"),
	  menu);
	new QListWidgetItem(QIcon::fromTheme(DEM_NAME, QIcon(DEM_ICON)), tr("DEM"),
	  menu);
	new QListWidgetItem(QIcon::fromTheme(POSITION_NAME, QIcon(POSITION_ICON)),
	  tr("Position"), menu);
	new QListWidgetItem(QIcon::fromTheme(PRINT_EXPORT_NAME,
	  QIcon(PRINT_EXPORT_ICON)), tr("Print & Export"), menu);
	new QListWidgetItem(QIcon::fromTheme(SYSTEM_NAME, QIcon(SYSTEM_ICON)),
	  tr("System"), menu);
#endif // Q_OS_ANDROID

	QHBoxLayout *contentLayout = new QHBoxLayout();
	contentLayout->addWidget(menu);
	contentLayout->addWidget(pages);

	menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	menu->setMaximumWidth(menu->sizeHintForColumn(0) + 2 * menu->frameWidth()
	  + MENU_MARGIN);
	pages->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	pages->setMinimumWidth(2 * menu->size().width());

	connect(menu, &QListWidget::currentRowChanged, pages,
	  &QStackedWidget::setCurrentIndex);
	menu->item(0)->setSelected(true);


	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
	  | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this,
	  &OptionsDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this,
	  &OptionsDialog::reject);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addLayout(contentLayout);
	layout->addWidget(buttonBox);
#ifdef Q_OS_MAC
	layout->setSizeConstraint(QLayout::SetFixedSize);
#endif // Q_OS_MAC
	setLayout(layout);

	setWindowTitle(tr("Options"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void OptionsDialog::accept()
{
	_options.palette.setColor(_baseColor->color());
	_options.palette.setShift(_colorOffset->value() / 100.0);
	_options.mapOpacity = _mapOpacity->value();
	_options.backgroundColor = _backgroundColor->color();
	_options.crosshairColor = _crosshairColor->color();
	_options.infoColor = _infoColor->color();
	_options.infoBackground = _infoBackground->isChecked();
	_options.trackWidth = _trackWidth->value();
	_options.trackStyle = (Qt::PenStyle) _trackStyle->itemData(
	  _trackStyle->currentIndex()).toInt();
	_options.routeWidth = _routeWidth->value();
	_options.routeStyle = (Qt::PenStyle) _routeStyle->itemData(
	  _routeStyle->currentIndex()).toInt();
	_options.pathAntiAliasing = _pathAA->isChecked();
	_options.areaWidth = _areaWidth->value();
	_options.areaStyle = (Qt::PenStyle) _areaStyle->itemData(
	  _areaStyle->currentIndex()).toInt();
	_options.areaOpacity = _areaOpacity->value();
	_options.waypointSize = _waypointSize->value();
	_options.waypointColor = _waypointColor->color();
	_options.poiSize = _poiSize->value();
	_options.poiColor = _poiColor->color();
	_options.graphWidth = _graphWidth->value();
	_options.sliderColor = _sliderColor->color();
	_options.graphAntiAliasing = _graphAA->isChecked();

	_options.outputProjection = _outputProjection->itemData(
	  _outputProjection->currentIndex()).toInt();
	_options.inputProjection = _inputProjection->itemData(
	  _inputProjection->currentIndex()).toInt();
	_options.hidpiMap = _hidpi->isChecked();

	_options.elevationFilter = _elevationFilter->value();
	_options.speedFilter = _speedFilter->value();
	_options.heartRateFilter = _heartRateFilter->value();
	_options.cadenceFilter = _cadenceFilter->value();
	_options.powerFilter = _powerFilter->value();
	_options.outlierEliminate = _outlierEliminate->isChecked();
	_options.detectPauses = _detectPauses->isChecked();
	_options.automaticPause = _automaticPause->isChecked();
	qreal pauseSpeed = (_units == Imperial)
		? _pauseSpeed->value() / MS2MIH : (_units == Nautical)
		? _pauseSpeed->value() / MS2KN : _pauseSpeed->value() / MS2KMH;
	if (qAbs(pauseSpeed - _options.pauseSpeed) > 0.01)
		_options.pauseSpeed = pauseSpeed;
	_options.pauseInterval = _pauseInterval->value();
	_options.useReportedSpeed = _reportedSpeed->isChecked();
	_options.dataUseDEM = _dataDEMElevation->isChecked();
	_options.showSecondaryElevation = _showSecondaryElevation->isChecked();
	_options.showSecondarySpeed = _showSecondarySpeed->isChecked();
	_options.timeZone.setType(_utcZone->isChecked()
	  ? TimeZoneInfo::UTC : _systemZone->isChecked()
	  ? TimeZoneInfo::System : TimeZoneInfo::Custom);
	_options.timeZone.setCustomZone(QTimeZone(_timeZone->currentText()
	  .toLatin1()));
	_options.useSegments = _useSegments->isChecked();

	qreal poiRadius = (_units == Imperial)
		? _poiRadius->value() * MIINM : (_units == Nautical)
		? _poiRadius->value() * NMIINM : _poiRadius->value() * KMINM;
	if (qAbs(poiRadius - _options.poiRadius) > 0.01)
		_options.poiRadius = poiRadius;

	_options.demURL = _demURL->text();
	_options.demAuthorization = _demAuth->isEnabled();
	_options.demUsername = _demAuth->username();
	_options.demPassword = _demAuth->password();
	_options.hillshadingAlpha = qRound((_hillshadingAlpha->value() / 100.0)
	  * 255);
	_options.hillshadingLightening = _hillshadingLightening->value() / 100.0;
	_options.hillshadingBlur = _hillshadingBlur->value();
	_options.hillshadingAzimuth = _hillshadingAzimuth->value();
	_options.hillshadingAltitude = _hillshadingAltitude->value();
	_options.hillshadingZFactor = _hillshadingZFactor->value();

	_options.plugin = _positionPlugin->currentText();
	_options.pluginParams = _pluginParameters->parameters();

	_options.useOpenGL = _useOpenGL->isChecked();
	_options.enableHTTP2 = _enableHTTP2->isChecked();
	_options.pixmapCache = _pixmapCache->value();
	_options.demCache = _demCache->value();
	_options.connectionTimeout = _connectionTimeout->value();
	_options.dataPath = _dataPath->dir();
	_options.mapsPath = _mapsPath->dir();
	_options.poiPath = _poiPath->dir();

	_options.hiresPrint = _hires->isChecked();
	_options.printName = _name->isChecked();
	_options.printDate = _date->isChecked();
	_options.printDistance = _distance->isChecked();
	_options.printTime = _time->isChecked();
	_options.printMovingTime = _movingTime->isChecked();
	_options.printItemCount = _itemCount->isChecked();
	_options.separateGraphPage = _separateGraphPage->isChecked();

	QDialog::accept();
}
