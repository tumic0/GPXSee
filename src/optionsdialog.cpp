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
#include "units.h"
#include "icons.h"
#include "colorbox.h"
#include "stylecombobox.h"
#include "optionsdialog.h"

#define MENU_MARGIN 20


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
	QGroupBox *trackBox = new QGroupBox(tr("Tracks"));
	QFormLayout *trackLayout = new QFormLayout();
	trackLayout->addRow(tr("Line width:"), _trackWidth);
	trackLayout->addRow(tr("Line style:"), _trackStyle);
	trackBox->setLayout(trackLayout);

	_routeWidth = new QSpinBox();
	_routeWidth->setValue(_options->routeWidth);
	_routeWidth->setMinimum(1);
	_routeStyle = new StyleComboBox();
	_routeStyle->setValue(_options->routeStyle);
	QGroupBox *routeBox = new QGroupBox(tr("Routes"));
	QFormLayout *routeLayout = new QFormLayout();
	routeLayout->addRow(tr("Line width:"), _routeWidth);
	routeLayout->addRow(tr("Line style:"), _routeStyle);
	routeBox->setLayout(routeLayout);

	_pathAA = new QCheckBox(tr("Use anti-aliasing"));
	_pathAA->setChecked(_options->pathAntiAliasing);

	QWidget *pathTab = new QWidget();
	QVBoxLayout *pathTabLayout = new QVBoxLayout();
	pathTabLayout->addWidget(trackBox);
	pathTabLayout->addWidget(routeBox);
	pathTabLayout->addWidget(_pathAA);
	pathTab->setLayout(pathTabLayout);


	_graphWidth = new QSpinBox();
	_graphWidth->setValue(_options->graphWidth);
	_graphWidth->setMinimum(1);
	QFormLayout *graphLayout = new QFormLayout();
	graphLayout->addRow(tr("Line width:"), _graphWidth);

	_graphAA = new QCheckBox(tr("Use anti-aliasing"));
	_graphAA->setChecked(_options->graphAntiAliasing);

	QWidget *graphTab = new QWidget();
	QVBoxLayout *graphTabLayout = new QVBoxLayout();
	graphTabLayout->addLayout(graphLayout);
	graphTabLayout->addWidget(_graphAA);
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
	_poiRadius->setValue(_options->poiRadius / 1000);
	_poiRadius->setSuffix(UNIT_SPACE + tr("km"));

	QFormLayout *poiLayout = new QFormLayout();
	poiLayout->addRow(tr("POI radius:"), _poiRadius);

	QWidget *poiTab = new QWidget();
	poiTab->setLayout(poiLayout);

	QTabWidget *poiPage = new QTabWidget();
	poiPage->addTab(poiTab, tr("POI"));

	return poiPage;
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
	pages->addWidget(createSystemPage());

	QListWidget *menu = new QListWidget();
	new QListWidgetItem(QIcon(QPixmap(APPEARANCE_ICON)), tr("Appearance"),
	  menu);
	new QListWidgetItem(QIcon(QPixmap(POI_ICON)), tr("POI"), menu);
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

	_options->poiRadius = _poiRadius->value() * 1000;

	_options->useOpenGL = _useOpenGL->isChecked();

	QDialog::accept();
}
