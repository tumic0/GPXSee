#include <QApplication>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QKeyEvent>
#include <QSignalMapper>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>
#include <QActionGroup>
#include <QAction>
#include <QLabel>
#include "config.h"
#include "icons.h"
#include "keys.h"
#include "gpx.h"
#include "map.h"
#include "maplist.h"
#include "elevationgraph.h"
#include "speedgraph.h"
#include "heartrategraph.h"
#include "trackview.h"
#include "infoitem.h"
#include "filebrowser.h"
#include "gui.h"


#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

struct GraphTab {
	GraphView *view;
	QString label;
};

static QString timeSpan(qreal time)
{
	unsigned h, m, s;

	h = time / 3600;
	m = (time - (h * 3600)) / 60;
	s = time - (h * 3600) - (m * 60);

	return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0'))
	  .arg(s, 2, 10, QChar('0'));
}


GUI::GUI(QWidget *parent) : QMainWindow(parent)
{
	loadMaps();
	loadPOIs();

	createActions();
	createMenus();
	createToolBars();
	createTrackView();
	createTrackGraphs();
	createStatusBar();

	connect(_elevationGraph, SIGNAL(sliderPositionChanged(qreal)), this,
	  SLOT(sliderPositionChanged(qreal)));
	connect(_speedGraph, SIGNAL(sliderPositionChanged(qreal)), this,
	  SLOT(sliderPositionChanged(qreal)));
	connect(_heartRateGraph, SIGNAL(sliderPositionChanged(qreal)), this,
	  SLOT(sliderPositionChanged(qreal)));

	_browser = new FileBrowser(this);
	_browser->setFilter(QStringList("*.gpx"));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(_track);
	layout->addWidget(_trackGraphs);

	QWidget *widget = new QWidget;
	widget->setLayout(layout);
	setCentralWidget(widget);

	setWindowTitle(APP_NAME);
	setUnifiedTitleAndToolBarOnMac(true);

	_distance = 0;
	_time = 0;
	_trackCount = 0;

	_sliderPos = 0;

	updateGraphTabs();
	updateTrackView();

	resize(600, 800);
}

void GUI::loadMaps()
{
	if (QFile::exists(USER_MAP_FILE))
		_maps = MapList::load(this, USER_MAP_FILE);
	else
		_maps = MapList::load(this, GLOBAL_MAP_FILE);
}

void GUI::loadPOIs()
{
	QFileInfoList list;
	QDir userDir(USER_POI_DIR);
	QDir globalDir(GLOBAL_POI_DIR);

	if (userDir.exists())
		list = userDir.entryInfoList(QStringList(), QDir::Files);
	else
		list = globalDir.entryInfoList(QStringList(), QDir::Files);

	for (int i = 0; i < list.size(); ++i)
		_poi.loadFile(list.at(i).absoluteFilePath());
}

void GUI::createMapActions()
{
	QActionGroup *ag = new QActionGroup(this);
	ag->setExclusive(true);

	QSignalMapper *sm = new QSignalMapper(this);

	for (int i = 0; i < _maps.count(); i++) {
		QAction *a = new QAction(_maps.at(i)->name(), this);
		a->setCheckable(true);
		a->setActionGroup(ag);

		sm->setMapping(a, i);
		connect(a, SIGNAL(triggered()), sm, SLOT(map()));

		_mapActions.append(a);
	}

	connect(sm, SIGNAL(mapped(int)), this, SLOT(mapChanged(int)));

	_mapActions.at(0)->setChecked(true);
	_currentMap = _maps.at(0);
}

void GUI::createPOIFilesActions()
{
	_poiFilesSM = new QSignalMapper(this);

	for (int i = 0; i < _poi.files().count(); i++)
		createPOIFileAction(i);

	connect(_poiFilesSM, SIGNAL(mapped(int)), this, SLOT(poiFileChecked(int)));
}

QAction *GUI::createPOIFileAction(int index)
{
	QAction *a = new QAction(QFileInfo(_poi.files().at(index)).fileName(),
	  this);
	a->setCheckable(true);
	a->setChecked(true);

	_poiFilesSM->setMapping(a, index);
	connect(a, SIGNAL(triggered()), _poiFilesSM, SLOT(map()));

	_poiFilesActions.append(a);

	return a;
}

void GUI::createActions()
{
	// Action Groups
	_fileActionGroup = new QActionGroup(this);
	_fileActionGroup->setExclusive(false);
	_fileActionGroup->setEnabled(false);

	_navigationActionGroup = new QActionGroup(this);
	_navigationActionGroup->setEnabled(false);

	// General actions
	_exitAction = new QAction(QIcon(QPixmap(QUIT_ICON)), tr("Quit"), this);
	_exitAction->setShortcut(QKeySequence::Quit);
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	// Help & About
	_dataSourcesAction = new QAction(tr("Data sources"), this);
	connect(_dataSourcesAction, SIGNAL(triggered()), this, SLOT(dataSources()));
	_keysAction = new QAction(tr("Keyboard controls"), this);
	connect(_keysAction, SIGNAL(triggered()), this, SLOT(keys()));
	_aboutAction = new QAction(QIcon(QPixmap(APP_ICON)),
	  tr("About GPXSee"), this);
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));
	_aboutQtAction = new QAction(QIcon(QPixmap(QT_ICON)), tr("About Qt"), this);
	connect(_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// File related actions
	_openFileAction = new QAction(QIcon(QPixmap(OPEN_FILE_ICON)),
	  tr("Open"), this);
	_openFileAction->setShortcut(QKeySequence::Open);
	connect(_openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));
	_saveFileAction = new QAction(QIcon(QPixmap(SAVE_FILE_ICON)),
	  tr("Save"), this);
	_saveFileAction->setShortcut(QKeySequence::Save);
	_saveFileAction->setActionGroup(_fileActionGroup);
	connect(_saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));
	_saveAsAction = new QAction(QIcon(QPixmap(SAVE_AS_ICON)),
	  tr("Save as"), this);
	_saveAsAction->setShortcut(QKeySequence::SaveAs);
	_saveAsAction->setActionGroup(_fileActionGroup);
	connect(_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
	_closeFileAction = new QAction(QIcon(QPixmap(CLOSE_FILE_ICON)),
	  tr("Close"), this);
	_closeFileAction->setShortcut(QKeySequence::Close);
	_closeFileAction->setActionGroup(_fileActionGroup);
	connect(_closeFileAction, SIGNAL(triggered()), this, SLOT(closeAll()));
	_reloadFileAction = new QAction(QIcon(QPixmap(RELOAD_FILE_ICON)),
	  tr("Reload"), this);
	_reloadFileAction->setShortcut(QKeySequence::Refresh);
	_reloadFileAction->setActionGroup(_fileActionGroup);
	connect(_reloadFileAction, SIGNAL(triggered()), this, SLOT(reloadFile()));

	// POI actions
	_openPOIAction = new QAction(QIcon(QPixmap(OPEN_FILE_ICON)),
	  tr("Load POI file"), this);
	connect(_openPOIAction, SIGNAL(triggered()), this, SLOT(openPOIFile()));
	_closePOIAction = new QAction(QIcon(QPixmap(CLOSE_FILE_ICON)),
	  tr("Close POI files"), this);
	connect(_closePOIAction, SIGNAL(triggered()), this, SLOT(closePOIFiles()));
	_showPOIAction = new QAction(QIcon(QPixmap(SHOW_POI_ICON)),
	  tr("Show POIs"), this);
	_showPOIAction->setCheckable(true);
	_showPOIAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
	connect(_showPOIAction, SIGNAL(triggered(bool)), this, SLOT(showPOI(bool)));
	createPOIFilesActions();

	// Map actions
	_showMapAction = new QAction(QIcon(QPixmap(SHOW_MAP_ICON)), tr("Show map"),
	  this);
	_showMapAction->setCheckable(true);
	_showMapAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
	connect(_showMapAction, SIGNAL(triggered(bool)), this, SLOT(showMap(bool)));
	_clearMapCacheAction = new QAction(tr("Clear tile cache"), this);
	connect(_clearMapCacheAction, SIGNAL(triggered()), this,
	  SLOT(clearMapCache()));
	if (_maps.empty()) {
		_showMapAction->setEnabled(false);
		_clearMapCacheAction->setEnabled(false);
	} else {
		createMapActions();
		_showMapAction->setChecked(true);
	}

	// Settings actions
	_showGraphsAction = new QAction(tr("Show graphs"), this);
	_showGraphsAction->setCheckable(true);
	_showGraphsAction->setChecked(true);
	_showGraphsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	connect(_showGraphsAction, SIGNAL(triggered(bool)), this,
	  SLOT(showGraphs(bool)));
	_showToolbarsAction = new QAction(tr("Show toolbars"), this);
	_showToolbarsAction->setCheckable(true);
	_showToolbarsAction->setChecked(true);
	connect(_showToolbarsAction, SIGNAL(triggered(bool)), this,
	  SLOT(showToolbars(bool)));
	QActionGroup *ag = new QActionGroup(this);
	ag->setExclusive(true);
	_metricUnitsAction = new QAction(tr("Metric"), this);
	_metricUnitsAction->setCheckable(true);
	_metricUnitsAction->setActionGroup(ag);
	_metricUnitsAction->setChecked(true);
	connect(_metricUnitsAction, SIGNAL(triggered()), this,
	  SLOT(setMetricUnits()));
	_imperialUnitsAction = new QAction(tr("Imperial"), this);
	_imperialUnitsAction->setCheckable(true);
	_imperialUnitsAction->setActionGroup(ag);
	connect(_imperialUnitsAction, SIGNAL(triggered()), this,
	  SLOT(setImperialUnits()));

	// Navigation actions
	_nextAction = new QAction(QIcon(QPixmap(NEXT_FILE_ICON)), tr("Next"), this);
	_nextAction->setActionGroup(_navigationActionGroup);
	connect(_nextAction, SIGNAL(triggered()), this, SLOT(next()));
	_prevAction = new QAction(QIcon(QPixmap(PREV_FILE_ICON)), tr("Previous"),
	  this);
	_prevAction->setActionGroup(_navigationActionGroup);
	connect(_prevAction, SIGNAL(triggered()), this, SLOT(prev()));
	_lastAction = new QAction(QIcon(QPixmap(LAST_FILE_ICON)), tr("Last"), this);
	_lastAction->setActionGroup(_navigationActionGroup);
	connect(_lastAction, SIGNAL(triggered()), this, SLOT(last()));
	_firstAction = new QAction(QIcon(QPixmap(FIRST_FILE_ICON)), tr("First"),
	  this);
	_firstAction->setActionGroup(_navigationActionGroup);
	connect(_firstAction, SIGNAL(triggered()), this, SLOT(first()));
}

void GUI::createMenus()
{
	_fileMenu = menuBar()->addMenu(tr("File"));
	_fileMenu->addAction(_openFileAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_saveFileAction);
	_fileMenu->addAction(_saveAsAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_reloadFileAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_closeFileAction);
#ifndef Q_OS_MAC
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);
#endif // Q_OS_MAC

	_mapMenu = menuBar()->addMenu(tr("Map"));
	_mapMenu->addActions(_mapActions);
	_mapMenu->addSeparator();
	_mapMenu->addAction(_clearMapCacheAction);
	_mapMenu->addSeparator();
	_mapMenu->addAction(_showMapAction);

	_poiMenu = menuBar()->addMenu(tr("POI"));
	_poiFilesMenu = _poiMenu->addMenu(tr("POI files"));
	_poiFilesMenu->addActions(_poiFilesActions);
	_poiMenu->addSeparator();
	_poiMenu->addAction(_openPOIAction);
	_poiMenu->addAction(_closePOIAction);
	_poiMenu->addSeparator();
	_poiMenu->addAction(_showPOIAction);

	_settingsMenu = menuBar()->addMenu(tr("Settings"));
	_unitsMenu = _settingsMenu->addMenu(tr("Units"));
	_unitsMenu->addAction(_metricUnitsAction);
	_unitsMenu->addAction(_imperialUnitsAction);
	_settingsMenu->addSeparator();
	_settingsMenu->addAction(_showToolbarsAction);
	_settingsMenu->addAction(_showGraphsAction);

	_helpMenu = menuBar()->addMenu(tr("Help"));
	_helpMenu->addAction(_dataSourcesAction);
	_helpMenu->addAction(_keysAction);
	_helpMenu->addSeparator();
	_helpMenu->addAction(_aboutAction);
	_helpMenu->addAction(_aboutQtAction);
}

void GUI::createToolBars()
{
	_fileToolBar = addToolBar(tr("File"));
	_fileToolBar->addAction(_openFileAction);
	_fileToolBar->addAction(_saveFileAction);
	_fileToolBar->addAction(_reloadFileAction);
	_fileToolBar->addAction(_closeFileAction);
#ifdef Q_OS_MAC
	_fileToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // Q_OS_MAC

	_showToolBar = addToolBar(tr("Show"));
	_showToolBar->addAction(_showPOIAction);
	_showToolBar->addAction(_showMapAction);
#ifdef Q_OS_MAC
	_showToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // Q_OS_MAC

	_navigationToolBar = addToolBar(tr("Navigation"));
	_navigationToolBar->addAction(_firstAction);
	_navigationToolBar->addAction(_prevAction);
	_navigationToolBar->addAction(_nextAction);
	_navigationToolBar->addAction(_lastAction);
#ifdef Q_OS_MAC
	_navigationToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // Q_OS_MAC
}

void GUI::createTrackView()
{
	_track = new TrackView(this);

	if (_showMapAction->isChecked())
		_track->setMap(_currentMap);
}

void GUI::createTrackGraphs()
{
	_elevationGraph = new ElevationGraph;
	_speedGraph = new SpeedGraph;
	_heartRateGraph = new HeartRateGraph;

	_trackGraphs = new QTabWidget;
	connect(_trackGraphs, SIGNAL(currentChanged(int)), this,
	  SLOT(graphChanged(int)));

	_trackGraphs->setFixedHeight(200);
	_trackGraphs->setSizePolicy(
		QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
}

void GUI::createStatusBar()
{
	_fileNameLabel = new QLabel();
	_distanceLabel = new QLabel();
	_timeLabel = new QLabel();
	_distanceLabel->setAlignment(Qt::AlignHCenter);
	_timeLabel->setAlignment(Qt::AlignHCenter);

	statusBar()->addPermanentWidget(_fileNameLabel, 8);
	statusBar()->addPermanentWidget(_distanceLabel, 1);
	statusBar()->addPermanentWidget(_timeLabel, 1);
	statusBar()->setSizeGripEnabled(false);

	updateStatusBarInfo();
}

void GUI::about()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("About GPXSee"));
	msgBox.setText(QString("<h3>") + QString(APP_NAME " " APP_VERSION)
	  + QString("</h3><p>") + tr("GPX viewer and analyzer") + QString("<p/>"));
	msgBox.setInformativeText(QString("<table width=\"300\"><tr><td>")
	  + tr("GPXSee is distributed under the terms of the GNU General Public "
	  "License version 3. For more info about GPXSee visit the project "
	  "homepage at ") + QString("<a href=\"" APP_HOMEPAGE "\">" APP_HOMEPAGE
	  "</a>.</td></tr></table>"));

	QIcon icon = msgBox.windowIcon();
	QSize size = icon.actualSize(QSize(64, 64));
	msgBox.setIconPixmap(icon.pixmap(size));

	msgBox.exec();
}

void GUI::keys()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("Keyboard controls"));
	msgBox.setText(QString("<h3>") + tr("Keyboard controls") + QString("</h3>"));
	msgBox.setInformativeText(
	  QString("<div><table width=\"300\"><tr><td>") + tr("Next file")
	  + QString("</td><td><i>SPACE</i></td></tr><tr><td>") + tr("Previous file")
	  + QString("</td><td><i>BACKSPACE</i></td></tr><tr><td>")
	  + tr("First file") + QString("</td><td><i>HOME</i></td></tr><tr><td>")
	  + tr("Last file") + QString("</td><td><i>END</i></td></tr><tr><td></td>"
	  "<td></td></tr><tr><td>") + tr("Append modifier")
	  + QString("</td><td><i>SHIFT</i></td></tr></table></div>"));

	msgBox.exec();
}

void GUI::dataSources()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("Data sources"));
	msgBox.setText(QString("<h3>") + tr("Data sources") + QString("</h3>"));
	msgBox.setInformativeText(
	  QString("<h4>") + tr("Map sources") + QString("</h4><p>")
	  + tr("Map (tiles) source URLs are read on program startup from the "
		"following file:")
		+ QString("</p><p><code>") + USER_MAP_FILE + QString("</code></p><p>")
		+ tr("The file format is one map entry per line, consisting of the map "
		  "name and tiles URL delimited by a TAB character. The tile X and Y "
		  "coordinates are replaced with $x and $y in the URL and the zoom "
		  "level is replaced with $z. An example map file could look like:")
		+ QString("</p><p><code>Map1	http://tile.server.com/map/$z/$x/$y.png"
		  "<br/>Map2	http://mapserver.org/map/$z-$x-$y</code></p>")

	  + QString("<h4>") + tr("POIs") + QString("</h4><p>")
	  + tr("To make GPXSee load a POI file automatically on startup, add "
		"the file to the following directory:")
		+ QString("</p><p><code>") + USER_POI_DIR + QString("</code></p>")
	);

	msgBox.exec();
}

void GUI::openFile()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Open file"),
	  QString(), tr("GPX files (*.gpx);;All files (*)"));
	QStringList list = files;

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		openFile(*it);
}

bool GUI::openFile(const QString &fileName)
{
	bool ret = true;

	if (fileName.isEmpty() || _files.contains(fileName))
		return false;

	if (loadFile(fileName)) {
		_files.append(fileName);
		_browser->setCurrent(fileName);
		_fileActionGroup->setEnabled(true);
		_navigationActionGroup->setEnabled(true);
	} else {
		if (_files.isEmpty())
			_fileActionGroup->setEnabled(false);
		ret = false;
	}

	updateNavigationActions();
	updateStatusBarInfo();
	updateGraphTabs();
	updateTrackView();

	return ret;
}

bool GUI::loadFile(const QString &fileName)
{
	GPX gpx;

	if (gpx.loadFile(fileName)) {
		_elevationGraph->loadGPX(gpx);
		_speedGraph->loadGPX(gpx);
		_heartRateGraph->loadGPX(gpx);
		updateGraphTabs();
		_track->setHidden(false);
		_track->loadGPX(gpx);
		if (_showPOIAction->isChecked())
			_track->loadPOI(_poi);
		_track->movePositionMarker(_sliderPos);

		for (int i = 0; i < gpx.trackCount(); i++) {
			_distance += gpx.track(i).distance();
			_time += gpx.track(i).time();
		}

		_trackCount += gpx.trackCount();

		return true;
	} else {
		QString error = fileName + QString("\n\n")
		  + tr("Error loading GPX file:\n%1").arg(gpx.errorString())
		  + QString("\n");
		if (gpx.errorLine())
			error.append(tr("Line: %1").arg(gpx.errorLine()));

		QMessageBox::critical(this, tr("Error"), error);
		return false;
	}
}

void GUI::openPOIFile()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Open POI file"),
	  QString(), tr("GPX files (*.gpx);;CSV files (*.csv);;All files (*)"));
	QStringList list = files;

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		openPOIFile(*it);
}

bool GUI::openPOIFile(const QString &fileName)
{
	if (fileName.isEmpty())
		return false;

	if (!_poi.loadFile(fileName)) {
		QString error = tr("Error loading POI file:\n%1")
		  .arg(_poi.errorString()) + QString("\n");
		if (_poi.errorLine())
			error.append(tr("Line: %1").arg(_poi.errorLine()));
		QMessageBox::critical(this, tr("Error"), error);

		return false;
	} else {
		_showPOIAction->setChecked(true);
		_track->loadPOI(_poi);
		_poiFilesMenu->addAction(createPOIFileAction(
		  _poi.files().indexOf(fileName)));

		return true;
	}
}

void GUI::closePOIFiles()
{
	_poiFilesMenu->clear();

	for (int i = 0; i < _poiFilesActions.count(); i++)
		delete _poiFilesActions[i];
	_poiFilesActions.clear();

	_track->clearPOI();

	_poi.clear();
}

void GUI::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Export to PDF",
	  QString(), "*.pdf");

	if (!fileName.isEmpty()) {
		saveFile(fileName);
		_saveFileName = fileName;
	}
}

void GUI::saveFile()
{
	if (_saveFileName.isEmpty())
		emit saveAs();
	else
		saveFile(_saveFileName);
}

void GUI::saveFile(const QString &fileName)
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setPageSize(QPrinter::A4);
	printer.setOrientation(_track->orientation());
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(fileName);

	QPainter p(&printer);

	_track->plot(&p, QRectF(0, 300, printer.width(), (0.80 * printer.height())
	  - 400));
	_elevationGraph->plot(&p,  QRectF(0, 0.80 * printer.height(),
	  printer.width(), printer.height() * 0.20));

	QGraphicsScene scene;
	InfoItem info;
	if (_imperialUnitsAction->isChecked()) {
		info.insert(tr("Distance"), QString::number(_distance * M2MI, 'f', 1)
		  + UNIT_SPACE + tr("mi"));
		info.insert(tr("Time"), timeSpan(_time));
		info.insert(tr("Ascent"), QString::number(_elevationGraph->ascent()
		  * M2FT, 'f', 0) + UNIT_SPACE + tr("ft"));
		info.insert(tr("Descent"), QString::number(_elevationGraph->descent()
		  * M2FT, 'f', 0) + UNIT_SPACE + tr("ft"));
		info.insert(tr("Maximum"), QString::number(_elevationGraph->max()
		  * M2FT, 'f', 0) + UNIT_SPACE + tr("ft"));
		info.insert(tr("Minimum"), QString::number(_elevationGraph->min()
		  * M2FT, 'f', 0) + UNIT_SPACE + tr("ft"));
	} else {
		info.insert(tr("Distance"), QString::number(_distance * M2KM, 'f', 1)
		  + UNIT_SPACE + tr("km"));
		info.insert(tr("Time"), timeSpan(_time));
		info.insert(tr("Ascent"), QString::number(_elevationGraph->ascent(),
		  'f', 0) + UNIT_SPACE + tr("m"));
		info.insert(tr("Descent"), QString::number(_elevationGraph->descent(),
		  'f', 0) + UNIT_SPACE + tr("m"));
		info.insert(tr("Maximum"), QString::number(_elevationGraph->max(), 'f',
		  0) + UNIT_SPACE + tr("m"));
		info.insert(tr("Minimum"), QString::number(_elevationGraph->min(), 'f',
		  0) + UNIT_SPACE + tr("m"));
	}
	scene.addItem(&info);
	scene.render(&p, QRectF(0, 0, printer.width(), 200));
}

void GUI::reloadFile()
{
	_distance = 0;
	_time = 0;
	_trackCount = 0;

	_elevationGraph->clear();
	_speedGraph->clear();
	_heartRateGraph->clear();
	_track->clear();

	for (int i = 0; i < _files.size(); i++) {
		if (!loadFile(_files.at(i))) {
			_files.removeAt(i);
			i--;
		}
	}

	updateStatusBarInfo();
	updateGraphTabs();
	updateTrackView();
	if (_files.isEmpty())
		_fileActionGroup->setEnabled(false);
	else
		_browser->setCurrent(_files.last());
}

void GUI::closeFiles()
{
	_distance = 0;
	_time = 0;
	_trackCount = 0;

	_sliderPos = 0;

	_elevationGraph->clear();
	_speedGraph->clear();
	_heartRateGraph->clear();
	_track->clear();

	_files.clear();
}

void GUI::closeAll()
{
	closeFiles();

	_fileActionGroup->setEnabled(false);
	updateStatusBarInfo();
	updateGraphTabs();
	updateTrackView();
}

void GUI::showPOI(bool checked)
{
	if (checked)
		_track->loadPOI(_poi);
	else
		_track->clearPOI();
}

void GUI::showMap(bool checked)
{
	if (checked)
		_track->setMap(_currentMap);
	else
		_track->setMap(0);
}

void GUI::showGraphs(bool checked)
{
	_trackGraphs->setHidden(!checked);
}

void GUI::showToolbars(bool checked)
{
	if (checked) {
		addToolBar(_fileToolBar);
		addToolBar(_showToolBar);
		addToolBar(_navigationToolBar);
		_fileToolBar->show();
		_showToolBar->show();
		_navigationToolBar->show();
	} else {
		removeToolBar(_fileToolBar);
		removeToolBar(_showToolBar);
		removeToolBar(_navigationToolBar);
	}
}

void GUI::clearMapCache()
{
	_currentMap->clearCache();
	_track->redraw();
}

void GUI::updateStatusBarInfo()
{
	if (_files.count() == 0) {
		_fileNameLabel->setText(tr("No GPX files loaded"));
		_distanceLabel->clear();
		_timeLabel->clear();
		return;
	} else if (_files.count() == 1)
		_fileNameLabel->setText(_files.at(0));
	else
		_fileNameLabel->setText(tr("%1 tracks").arg(_trackCount));

	if (_imperialUnitsAction->isChecked()) {
		if (_distance < MIINM)
			_distanceLabel->setText(QString::number(_distance * M2FT, 'f', 0)
			  + UNIT_SPACE + tr("ft"));
		else
			_distanceLabel->setText(QString::number(_distance * M2MI, 'f', 1)
			  + UNIT_SPACE + tr("mi"));
	} else {
		if (_distance < KMINM)
			_distanceLabel->setText(QString::number(_distance, 'f', 0)
			  + UNIT_SPACE + tr("m"));
		else
			_distanceLabel->setText(QString::number(_distance * M2KM, 'f', 1)
			  + UNIT_SPACE + tr("km"));
	}
	_timeLabel->setText(timeSpan(_time));
}

void GUI::mapChanged(int index)
{
	_currentMap = _maps.at(index);

	if (_showMapAction->isChecked())
		_track->setMap(_currentMap);
}

void GUI::poiFileChecked(int index)
{
	_poi.enableFile(_poi.files().at(index),
	  _poiFilesActions.at(index)->isChecked());

	_track->clearPOI();
	if (_showPOIAction->isChecked())
		_track->loadPOI(_poi);
}

void GUI::sliderPositionChanged(qreal pos)
{
	_sliderPos = pos;
	_track->movePositionMarker(_sliderPos);
}

void GUI::graphChanged(int index)
{
	if (index < 0)
		return;

	GraphView *gv = static_cast<GraphView*>(_trackGraphs->widget(index));
	gv->setSliderPosition(_sliderPos);
}

void GUI::updateNavigationActions()
{
	if (_browser->isLast()) {
		_nextAction->setEnabled(false);
		_lastAction->setEnabled(false);
	} else {
		_nextAction->setEnabled(true);
		_lastAction->setEnabled(true);
	}

	if (_browser->isFirst()) {
		_prevAction->setEnabled(false);
		_firstAction->setEnabled(false);
	} else {
		_prevAction->setEnabled(true);
		_firstAction->setEnabled(true);
	}
}

void GUI::updateGraphTabs()
{
	struct GraphTab tabs[] = {
	  {_elevationGraph, tr("Elevation")},
	  {_speedGraph, tr("Speed")},
	  {_heartRateGraph, tr("Heart rate")}
	};
	int index;
	GraphView *gv;

	for (int i = 0; i < (int)ARRAY_SIZE(tabs); i++) {
		gv = tabs[i].view;
		if (!gv->count() && (index = _trackGraphs->indexOf(gv)) >= 0)
			_trackGraphs->removeTab(index);
	}

	for (int i = 0; i < (int)ARRAY_SIZE(tabs); i++) {
		gv = tabs[i].view;
		if (gv->count() && _trackGraphs->indexOf(gv) < 0)
			_trackGraphs->insertTab(i, gv, tabs[i].label);
	}

	for (int i = 0; i < (int)ARRAY_SIZE(tabs); i++) {
		if (tabs[i].view->count()) {
			if (_showGraphsAction->isChecked())
				_trackGraphs->setHidden(false);
			_showGraphsAction->setEnabled(true);
			return;
		}
	}

	_trackGraphs->setHidden(true);
	_showGraphsAction->setEnabled(false);
}

void GUI::updateTrackView()
{
	_track->setHidden(!(_track->trackCount() + _track->waypointCount()));
}

void GUI::setMetricUnits()
{
	_track->setUnits(Metric);
	_elevationGraph->setUnits(Metric);
	_speedGraph->setUnits(Metric);
	_heartRateGraph->setUnits(Metric);
	updateStatusBarInfo();
}

void GUI::setImperialUnits()
{
	_track->setUnits(Imperial);
	_elevationGraph->setUnits(Imperial);
	_speedGraph->setUnits(Imperial);
	_heartRateGraph->setUnits(Imperial);
	updateStatusBarInfo();
}

void GUI::next()
{
	QString file = _browser->next();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file);
}

void GUI::prev()
{
	QString file = _browser->prev();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file);
}

void GUI::last()
{
	QString file = _browser->last();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file);
}

void GUI::first()
{
	QString file = _browser->first();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file);
}

void GUI::keyPressEvent(QKeyEvent *event)
{
	QString file;

	switch (event->key()) {
		case PREV_KEY:
			file = _browser->prev();
			break;
		case NEXT_KEY:
			file = _browser->next();
			break;
		case FIRST_KEY:
			file = _browser->first();
			break;
		case LAST_KEY:
			file = _browser->last();
			break;
	}

	if (!file.isNull()) {
		if (!(event->modifiers() & MODIFIER))
			closeFiles();
		openFile(file);
	}
}
