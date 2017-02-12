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
#include <QSysInfo>
#include "config.h"
#include "icons.h"
#include "colorbox.h"
#include "stylecombobox.h"
#include "optionsdialog.h"

#define MENU_MARGIN 20
#define MENU_ICON_SIZE 32


QWidget *OptionsDialog::createAppearancePage()
{
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

	QWidget *colorTab = new QWidget();
	colorTab->setLayout(paletteLayout);


	_trackWidth = new QSpinBox();
	_trackWidth->setValue(_options->trackWidth);
	_trackWidth->setMinimum(1);
	_trackStyle = new StyleComboBox();
	_trackStyle->setValue(_options->trackStyle);
	QFormLayout *trackLayout = new QFormLayout();
	trackLayout->addRow(tr("Track width:"), _trackWidth);
	trackLayout->addRow(tr("Track style:"), _trackStyle);
#ifndef Q_OS_MAC
	QGroupBox *trackBox = new QGroupBox(tr("Tracks"));
	trackBox->setLayout(trackLayout);
#endif

	_routeWidth = new QSpinBox();
	_routeWidth->setValue(_options->routeWidth);
	_routeWidth->setMinimum(1);
	_routeStyle = new StyleComboBox();
	_routeStyle->setValue(_options->routeStyle);
	QFormLayout *routeLayout = new QFormLayout();
	routeLayout->addRow(tr("Route width:"), _routeWidth);
	routeLayout->addRow(tr("Route style:"), _routeStyle);
#ifndef Q_OS_MAC
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
	QFrame *l1 = new QFrame();
	l1->setFrameShape(QFrame::HLine);
	l1->setFrameShadow(QFrame::Sunken);
	QFrame *l2 = new QFrame();
	l2->setFrameShape(QFrame::HLine);
	l2->setFrameShadow(QFrame::Sunken);

	pathTabLayout->addLayout(trackLayout);
	pathTabLayout->addWidget(l1);
	pathTabLayout->addLayout(routeLayout);
	pathTabLayout->addWidget(l2);
#else
	pathTabLayout->addWidget(trackBox);
	pathTabLayout->addWidget(routeBox);
#endif
	pathTabLayout->addLayout(pathAALayout);
	pathTab->setLayout(pathTabLayout);

	_graphWidth = new QSpinBox();
	_graphWidth->setValue(_options->graphWidth);
	_graphWidth->setMinimum(1);
	QFormLayout *graphLayout = new QFormLayout();
	graphLayout->addRow(tr("Line width:"), _graphWidth);

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

	QTabWidget *appearancePage = new QTabWidget();
	appearancePage->addTab(colorTab, tr("Colors"));
	appearancePage->addTab(pathTab, tr("Paths"));
	appearancePage->addTab(graphTab, tr("Graphs"));

	return appearancePage;
}

QWidget *OptionsDialog::createPOIPage()
{
	_poiRadius = new QDoubleSpinBox();
	_poiRadius->setSingleStep(1);
	_poiRadius->setDecimals(1);
	if (_options->units == Imperial) {
		_poiRadius->setValue(_options->poiRadius / MIINM);
		_poiRadius->setSuffix(UNIT_SPACE + tr("mi"));
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
	exportPage->addTab(headerTab, tr("Header"));
	exportPage->addTab(graphTab, tr("Graphs"));

	return exportPage;
}

QWidget *OptionsDialog::createSystemPage()
{
	_useOpenGL = new QCheckBox(tr("Use OpenGL"));
#ifdef Q_OS_WIN32
	if (QSysInfo::WindowsVersion < QSysInfo::WV_VISTA) {
		_useOpenGL->setChecked(false);
		_useOpenGL->setEnabled(false);
	} else
#endif // Q_OS_WIN32
	_useOpenGL->setChecked(_options->useOpenGL);

	QFormLayout *systemLayout = new QFormLayout();
	systemLayout->addWidget(_useOpenGL);

	QWidget *systemTab = new QWidget();
	systemTab->setLayout(systemLayout);

	QTabWidget *systemPage = new QTabWidget();
	systemPage->addTab(systemTab, tr("System"));

	return systemPage;
}

OptionsDialog::OptionsDialog(Options *options, QWidget *parent)
  : QDialog(parent), _options(options)
{
	QStackedWidget *pages = new QStackedWidget();
	pages->addWidget(createAppearancePage());
	pages->addWidget(createPOIPage());
	pages->addWidget(createExportPage());
	pages->addWidget(createSystemPage());

	QListWidget *menu = new QListWidget();
	menu->setIconSize(QSize(MENU_ICON_SIZE, MENU_ICON_SIZE));
	new QListWidgetItem(QIcon(QPixmap(APPEARANCE_ICON)), tr("Appearance"),
	  menu);
	new QListWidgetItem(QIcon(QPixmap(POI_ICON)), tr("POI"), menu);
	new QListWidgetItem(QIcon(QPixmap(PRINT_EXPORT_ICON)), tr("Print & Export"),
	  menu);
	new QListWidgetItem(QIcon(QPixmap(SYSTEM_ICON)), tr("System"), menu);

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
	_options->trackWidth = _trackWidth->value();
	_options->trackStyle = (Qt::PenStyle) _trackStyle->itemData(
	  _trackStyle->currentIndex()).toInt();
	_options->routeWidth = _routeWidth->value();
	_options->routeStyle = (Qt::PenStyle) _routeStyle->itemData(
	  _routeStyle->currentIndex()).toInt();
	_options->pathAntiAliasing = _pathAA->isChecked();
	_options->graphWidth = _graphWidth->value();
	_options->graphAntiAliasing = _graphAA->isChecked();

	if (_options->units == Imperial)
		_options->poiRadius = _poiRadius->value() * MIINM;
	else
		_options->poiRadius = _poiRadius->value() * KMINM;

	_options->useOpenGL = _useOpenGL->isChecked();

	_options->printName = _name->isChecked();
	_options->printDate = _date->isChecked();
	_options->printDistance = _distance->isChecked();
	_options->printTime = _time->isChecked();
	_options->printMovingTime = _movingTime->isChecked();
	_options->printItemCount = _itemCount->isChecked();
	_options->separateGraphPage = _separateGraphPage->isChecked();

	QDialog::accept();
}
