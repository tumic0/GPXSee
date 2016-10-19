#include <QApplication>
#include <QSplitter>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPainter>
#include <QPaintEngine>
#include <QPaintDevice>
#include <QKeyEvent>
#include <QSignalMapper>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>
#include <QActionGroup>
#include <QAction>
#include <QLabel>
#include <QSettings>
#include <QLocale>
#include <QMimeData>
#include <QUrl>
#include <QPixmapCache>
#include "config.h"
#include "icons.h"
#include "keys.h"
#include "settings.h"
#include "gpx.h"
#include "map.h"
#include "maplist.h"
#include "elevationgraph.h"
#include "speedgraph.h"
#include "heartrategraph.h"
#include "temperaturegraph.h"
#include "pathview.h"
#include "trackinfo.h"
#include "filebrowser.h"
#include "cpuarch.h"
#include "exportdialog.h"
#include "graphtab.h"
#include "misc.h"
#include "gui.h"


GUI::GUI(QWidget *parent) : QMainWindow(parent)
{
	loadMaps();
	loadPOIs();

	createPathView();
	createGraphTabs();
	createStatusBar();
	createActions();
	createMenus();
	createToolBars();

	_browser = new FileBrowser(this);
	_browser->setFilter(QStringList("*.gpx"));

	QSplitter *splitter = new QSplitter();
	splitter->setOrientation(Qt::Vertical);
	splitter->setChildrenCollapsible(false);
	splitter->addWidget(_pathView);
	splitter->addWidget(_graphTabWidget);
	splitter->setContentsMargins(0, 0, 0, 0);
	splitter->setStretchFactor(0, 255);
	splitter->setStretchFactor(1, 1);
	setCentralWidget(splitter);

	setWindowIcon(QIcon(QPixmap(APP_ICON)));
	setWindowTitle(APP_NAME);
	setUnifiedTitleAndToolBarOnMac(true);

	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;

	_sliderPos = 0;

	updateGraphTabs();
	updateTrackView();
	updateStatusBarInfo();

	readSettings();

	_exportPaperSize = (QLocale::system().measurementSystem()
	  == QLocale::ImperialSystem) ? QPrinter::Letter : QPrinter::A4;
	_exportOrientation = QPrinter::Portrait;
	_exportFileName = QString("%1/export.pdf").arg(QDir::currentPath());
	_exportMargins = MarginsF(5.0, 5.0, 5.0, 5.0);

	setAcceptDrops(true);
	QPixmapCache::setCacheLimit(65536);
}

GUI::~GUI()
{
	for (int i = 0; i < _tabs.size(); i++) {
		if (_graphTabWidget->indexOf(_tabs.at(i)) < 0)
			delete _tabs.at(i);
	}
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

	_poi = new POI(this);

	if (userDir.exists())
		list = userDir.entryInfoList(QStringList(), QDir::Files);
	else
		list = globalDir.entryInfoList(QStringList(), QDir::Files);

	for (int i = 0; i < list.size(); ++i)
		_poi->loadFile(list.at(i).absoluteFilePath());
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
}

void GUI::createPOIFilesActions()
{
	_poiFilesSM = new QSignalMapper(this);

	for (int i = 0; i < _poi->files().count(); i++)
		createPOIFileAction(i);

	connect(_poiFilesSM, SIGNAL(mapped(int)), this, SLOT(poiFileChecked(int)));
}

QAction *GUI::createPOIFileAction(int index)
{
	QAction *a = new QAction(QFileInfo(_poi->files().at(index)).fileName(),
	  this);
	a->setCheckable(true);

	_poiFilesSM->setMapping(a, index);
	connect(a, SIGNAL(triggered()), _poiFilesSM, SLOT(map()));

	_poiFilesActions.append(a);

	return a;
}

void GUI::createActions()
{
	QActionGroup *ag;


	// Action Groups
	_fileActionGroup = new QActionGroup(this);
	_fileActionGroup->setExclusive(false);
	_fileActionGroup->setEnabled(false);

	_navigationActionGroup = new QActionGroup(this);
	_navigationActionGroup->setEnabled(false);

	// General actions
	_exitAction = new QAction(QIcon(QPixmap(QUIT_ICON)), tr("Quit"), this);
	_exitAction->setShortcut(QUIT_SHORTCUT);
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));
	addAction(_exitAction);

	// Help & About
	_dataSourcesAction = new QAction(tr("Data sources"), this);
	connect(_dataSourcesAction, SIGNAL(triggered()), this, SLOT(dataSources()));
	_keysAction = new QAction(tr("Keyboard controls"), this);
	connect(_keysAction, SIGNAL(triggered()), this, SLOT(keys()));
	_aboutAction = new QAction(QIcon(QPixmap(APP_ICON)),
	  tr("About GPXSee"), this);
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	// File actions
	_openFileAction = new QAction(QIcon(QPixmap(OPEN_FILE_ICON)),
	  tr("Open"), this);
	_openFileAction->setShortcut(OPEN_SHORTCUT);
	connect(_openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));
	addAction(_openFileAction);
	_printFileAction = new QAction(QIcon(QPixmap(PRINT_FILE_ICON)),
	  tr("Print..."), this);
	_printFileAction->setActionGroup(_fileActionGroup);
	connect(_printFileAction, SIGNAL(triggered()), this, SLOT(printFile()));
	addAction(_printFileAction);
	_exportFileAction = new QAction(QIcon(QPixmap(EXPORT_FILE_ICON)),
	  tr("Export to PDF..."), this);
	_exportFileAction->setShortcut(EXPORT_SHORTCUT);
	_exportFileAction->setActionGroup(_fileActionGroup);
	connect(_exportFileAction, SIGNAL(triggered()), this, SLOT(exportFile()));
	addAction(_exportFileAction);
	_closeFileAction = new QAction(QIcon(QPixmap(CLOSE_FILE_ICON)),
	  tr("Close"), this);
	_closeFileAction->setShortcut(CLOSE_SHORTCUT);
	_closeFileAction->setActionGroup(_fileActionGroup);
	connect(_closeFileAction, SIGNAL(triggered()), this, SLOT(closeAll()));
	addAction(_closeFileAction);
	_reloadFileAction = new QAction(QIcon(QPixmap(RELOAD_FILE_ICON)),
	  tr("Reload"), this);
	_reloadFileAction->setShortcut(RELOAD_SHORTCUT);
	_reloadFileAction->setActionGroup(_fileActionGroup);
	connect(_reloadFileAction, SIGNAL(triggered()), this, SLOT(reloadFile()));
	addAction(_reloadFileAction);

	// POI actions
	_openPOIAction = new QAction(QIcon(QPixmap(OPEN_FILE_ICON)),
	  tr("Load POI file"), this);
	connect(_openPOIAction, SIGNAL(triggered()), this, SLOT(openPOIFile()));
	_closePOIAction = new QAction(QIcon(QPixmap(CLOSE_FILE_ICON)),
	  tr("Close POI files"), this);
	connect(_closePOIAction, SIGNAL(triggered()), this, SLOT(closePOIFiles()));
	_overlapPOIAction = new QAction(tr("Overlap POIs"), this);
	_overlapPOIAction->setCheckable(true);
	connect(_overlapPOIAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(setPOIOverlap(bool)));
	_showPOILabelsAction = new QAction(tr("Show POI labels"), this);
	_showPOILabelsAction->setCheckable(true);
	connect(_showPOILabelsAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(showPOILabels(bool)));
	_showPOIAction = new QAction(QIcon(QPixmap(SHOW_POI_ICON)),
	  tr("Show POIs"), this);
	_showPOIAction->setCheckable(true);
	_showPOIAction->setShortcut(SHOW_POI_SHORTCUT);
	connect(_showPOIAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(showPOI(bool)));
	addAction(_showPOIAction);
	createPOIFilesActions();

	// Map actions
	_showMapAction = new QAction(QIcon(QPixmap(SHOW_MAP_ICON)), tr("Show map"),
	  this);
	_showMapAction->setCheckable(true);
	_showMapAction->setShortcut(SHOW_MAP_SHORTCUT);
	connect(_showMapAction, SIGNAL(triggered(bool)), this, SLOT(showMap(bool)));
	addAction(_showMapAction);
	_clearMapCacheAction = new QAction(tr("Clear tile cache"), this);
	connect(_clearMapCacheAction, SIGNAL(triggered()), this,
	  SLOT(clearMapCache()));
	if (_maps.empty()) {
		_showMapAction->setEnabled(false);
		_clearMapCacheAction->setEnabled(false);
	} else {
		createMapActions();

		_nextMapAction = new QAction(tr("Next map"), this);
		_nextMapAction->setShortcut(NEXT_MAP_SHORTCUT);
		connect(_nextMapAction, SIGNAL(triggered()), this, SLOT(nextMap()));
		addAction(_nextMapAction);
		_prevMapAction = new QAction(tr("Next map"), this);
		_prevMapAction->setShortcut(PREV_MAP_SHORTCUT);
		connect(_prevMapAction, SIGNAL(triggered()), this, SLOT(prevMap()));
		addAction(_prevMapAction);
	}

	// Data actions
	_showTracksAction = new QAction(tr("Show tracks"), this);
	_showTracksAction->setCheckable(true);
	connect(_showTracksAction, SIGNAL(triggered(bool)), this,
	  SLOT(showTracks(bool)));
	_showRoutesAction = new QAction(tr("Show routes"), this);
	_showRoutesAction->setCheckable(true);
	connect(_showRoutesAction, SIGNAL(triggered(bool)), this,
	  SLOT(showRoutes(bool)));
	_showWaypointsAction = new QAction(tr("Show waypoints"), this);
	_showWaypointsAction->setCheckable(true);
	connect(_showWaypointsAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(showWaypoints(bool)));
	_showWaypointLabelsAction = new QAction(tr("Waypoint labels"), this);
	_showWaypointLabelsAction->setCheckable(true);
	connect(_showWaypointLabelsAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(showWaypointLabels(bool)));
	_showRouteWaypointsAction = new QAction(tr("Route waypoints"), this);
	_showRouteWaypointsAction->setCheckable(true);
	connect(_showRouteWaypointsAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(showRouteWaypoints(bool)));

	// Graph actions
	_showGraphsAction = new QAction(QIcon(QPixmap(SHOW_GRAPHS_ICON)),
	  tr("Show graphs"), this);
	_showGraphsAction->setCheckable(true);
	_showGraphsAction->setShortcut(SHOW_GRAPHS_SHORTCUT);
	connect(_showGraphsAction, SIGNAL(triggered(bool)), this,
	  SLOT(showGraphs(bool)));
	addAction(_showGraphsAction);
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	_distanceGraphAction = new QAction(tr("Distance"), this);
	_distanceGraphAction->setCheckable(true);
	_distanceGraphAction->setActionGroup(ag);
	_distanceGraphAction->setShortcut(DISTANCE_GRAPH_SHORTCUT);
	connect(_distanceGraphAction, SIGNAL(triggered()), this,
	  SLOT(setDistanceGraph()));
	addAction(_distanceGraphAction);
	_timeGraphAction = new QAction(tr("Time"), this);
	_timeGraphAction->setCheckable(true);
	_timeGraphAction->setActionGroup(ag);
	_timeGraphAction->setShortcut(TIME_GRAPH_SHORTCUT);
	connect(_timeGraphAction, SIGNAL(triggered()), this,
	  SLOT(setTimeGraph()));
	addAction(_timeGraphAction);
	_showGraphGridAction = new QAction(tr("Show grid"), this);
	_showGraphGridAction->setCheckable(true);
	connect(_showGraphGridAction, SIGNAL(triggered(bool)), this,
	  SLOT(showGraphGrids(bool)));

	// Settings actions
	_showToolbarsAction = new QAction(tr("Show toolbars"), this);
	_showToolbarsAction->setCheckable(true);
	connect(_showToolbarsAction, SIGNAL(triggered(bool)), this,
	  SLOT(showToolbars(bool)));
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	_metricUnitsAction = new QAction(tr("Metric"), this);
	_metricUnitsAction->setCheckable(true);
	_metricUnitsAction->setActionGroup(ag);
	connect(_metricUnitsAction, SIGNAL(triggered()), this,
	  SLOT(setMetricUnits()));
	_imperialUnitsAction = new QAction(tr("Imperial"), this);
	_imperialUnitsAction->setCheckable(true);
	_imperialUnitsAction->setActionGroup(ag);
	connect(_imperialUnitsAction, SIGNAL(triggered()), this,
	  SLOT(setImperialUnits()));
	_fullscreenAction = new QAction(QIcon(QPixmap(FULLSCREEN_ICON)),
	  tr("Fullscreen mode"), this);
	_fullscreenAction->setCheckable(true);
	_fullscreenAction->setShortcut(FULLSCREEN_SHORTCUT);
	connect(_fullscreenAction, SIGNAL(triggered(bool)), this,
	  SLOT(showFullscreen(bool)));
	addAction(_fullscreenAction);

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
	QMenu *fileMenu = menuBar()->addMenu(tr("File"));
	fileMenu->addAction(_openFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_printFileAction);
	fileMenu->addAction(_exportFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_reloadFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_closeFileAction);
#ifndef Q_OS_MAC
	fileMenu->addSeparator();
	fileMenu->addAction(_exitAction);
#endif // Q_OS_MAC

	QMenu *mapMenu = menuBar()->addMenu(tr("Map"));
	mapMenu->addActions(_mapActions);
	mapMenu->addSeparator();
	mapMenu->addAction(_clearMapCacheAction);
	mapMenu->addSeparator();
	mapMenu->addAction(_showMapAction);

	QMenu *graphMenu = menuBar()->addMenu(tr("Graph"));
	graphMenu->addAction(_distanceGraphAction);
	graphMenu->addAction(_timeGraphAction);
	graphMenu->addSeparator();
	graphMenu->addAction(_showGraphGridAction);
	graphMenu->addSeparator();
	graphMenu->addAction(_showGraphsAction);

	QMenu *poiMenu = menuBar()->addMenu(tr("POI"));
	_poiFilesMenu = poiMenu->addMenu(tr("POI files"));
	_poiFilesMenu->addActions(_poiFilesActions);
	poiMenu->addSeparator();
	poiMenu->addAction(_openPOIAction);
	poiMenu->addAction(_closePOIAction);
	poiMenu->addSeparator();
	poiMenu->addAction(_showPOILabelsAction);
	poiMenu->addAction(_overlapPOIAction);
	poiMenu->addSeparator();
	poiMenu->addAction(_showPOIAction);

	QMenu *dataMenu = menuBar()->addMenu(tr("Data"));
	QMenu *displayMenu = dataMenu->addMenu(tr("Display"));
	displayMenu->addAction(_showWaypointLabelsAction);
	displayMenu->addAction(_showRouteWaypointsAction);
	dataMenu->addSeparator();
	dataMenu->addAction(_showTracksAction);
	dataMenu->addAction(_showRoutesAction);
	dataMenu->addAction(_showWaypointsAction);

	QMenu *settingsMenu = menuBar()->addMenu(tr("Settings"));
	QMenu *unitsMenu = settingsMenu->addMenu(tr("Units"));
	unitsMenu->addAction(_metricUnitsAction);
	unitsMenu->addAction(_imperialUnitsAction);
	settingsMenu->addSeparator();
	settingsMenu->addAction(_showToolbarsAction);
	settingsMenu->addSeparator();
	settingsMenu->addAction(_fullscreenAction);

	QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
	helpMenu->addAction(_dataSourcesAction);
	helpMenu->addAction(_keysAction);
	helpMenu->addSeparator();
	helpMenu->addAction(_aboutAction);
}

void GUI::createToolBars()
{
#ifdef Q_OS_MAC
	setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // Q_OS_MAC

	_fileToolBar = addToolBar(tr("File"));
	_fileToolBar->addAction(_openFileAction);
	_fileToolBar->addAction(_reloadFileAction);
	_fileToolBar->addAction(_closeFileAction);
	_fileToolBar->addAction(_printFileAction);

	_showToolBar = addToolBar(tr("Show"));
	_showToolBar->addAction(_showPOIAction);
	_showToolBar->addAction(_showMapAction);
	_showToolBar->addAction(_showGraphsAction);

	_navigationToolBar = addToolBar(tr("Navigation"));
	_navigationToolBar->addAction(_firstAction);
	_navigationToolBar->addAction(_prevAction);
	_navigationToolBar->addAction(_nextAction);
	_navigationToolBar->addAction(_lastAction);
}

void GUI::createPathView()
{
	_pathView = new PathView(this);
	_pathView->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,
	  QSizePolicy::Expanding));
	_pathView->setMinimumHeight(200);
#ifdef Q_OS_WIN32
	_pathView->setFrameShape(QFrame::NoFrame);
#endif // Q_OS_WIN32

	_pathView->setPOI(_poi);
}

void GUI::createGraphTabs()
{
	_graphTabWidget = new QTabWidget();
	connect(_graphTabWidget, SIGNAL(currentChanged(int)), this,
	  SLOT(graphChanged(int)));

	_graphTabWidget->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,
	  QSizePolicy::Preferred));
	_graphTabWidget->setMinimumHeight(200);
#ifdef Q_OS_WIN32
	_graphTabWidget->setDocumentMode(true);
#endif // Q_OS_WIN32

	_tabs.append(new ElevationGraph);
	_tabs.append(new SpeedGraph);
	_tabs.append(new HeartRateGraph);
	_tabs.append(new TemperatureGraph);

	for (int i = 0; i < _tabs.count(); i++)
		connect(_tabs.at(i), SIGNAL(sliderPositionChanged(qreal)), this,
		  SLOT(sliderPositionChanged(qreal)));
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
}

void GUI::about()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("About GPXSee"));
	msgBox.setText(QString("<h2>") + QString(APP_NAME) + QString("</h2><p>")
	  + QString("<p>") + tr("Version ") + APP_VERSION + QString(" (")
	  + CPU_ARCH + QString(", Qt ") + QString(QT_VERSION_STR)
	  + QString(")</p>"));
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
	  + tr("Last file") + QString("</td><td><i>END</i></td></tr><tr><td>")
	  + tr("Append modifier") + QString("</td><td><i>SHIFT</i></td></tr>"
	  "<tr><td></td><td></td></tr><tr><td>")
	  + tr("Next map") + QString("</td><td><i>")
	  + _nextMapAction->shortcut().toString() + QString("</i></td></tr><tr><td>")
	  + tr("Previous map") + QString("</td><td><i>")
	  + _prevMapAction->shortcut().toString() + QString("</i></td></tr>"
	  "</table></div>"));

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
	updateWindowTitle();
	updateGraphTabs();
	updateTrackView();

	return ret;
}

bool GUI::loadFile(const QString &fileName)
{
	GPX gpx;
	QList<PathItem*> paths;

	if (gpx.loadFile(fileName)) {
		paths = _pathView->loadGPX(gpx);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->loadGPX(gpx, paths);
		updateGraphTabs();
		_pathView->setHidden(false);

		for (int i = 0; i < gpx.tracks().count(); i++) {
			_trackDistance += gpx.tracks().at(i)->distance();
			_time += gpx.tracks().at(i)->time();
			const QDate &date = gpx.tracks().at(i)->date().date();
			if (_dateRange.first.isNull() || _dateRange.first > date)
				_dateRange.first = date;
			if (_dateRange.second.isNull() || _dateRange.second < date)
				_dateRange.second = date;
		}
		_trackCount += gpx.tracks().count();

		for (int i = 0; i < gpx.routes().count(); i++)
			_routeDistance += gpx.routes().at(i)->distance();
		_routeCount += gpx.routes().count();

		_waypointCount += gpx.waypoints().count();

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
	  QString(), tr("All POI files (*.gpx *.csv)") + ";;"
	  + tr("GPX files (*.gpx)") + ";;" + tr("CSV files (*.csv)") + ";;"
	  + tr("All files (*)"));
	QStringList list = files;

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		openPOIFile(*it);
}

bool GUI::openPOIFile(const QString &fileName)
{
	if (fileName.isEmpty() || _poi->files().contains(fileName))
		return false;

	if (!_poi->loadFile(fileName)) {
		QString error = fileName + QString("\n\n")
		  + tr("Error loading POI file:\n%1").arg(_poi->errorString())
		  + QString("\n");
		if (_poi->errorLine())
			error.append(tr("Line: %1").arg(_poi->errorLine()));
		QMessageBox::critical(this, tr("Error"), error);

		return false;
	} else {
		_pathView->showPOI(true);
		_showPOIAction->setChecked(true);
		QAction *action = createPOIFileAction(_poi->files().indexOf(fileName));
		action->setChecked(true);
		_poiFilesMenu->addAction(action);

		return true;
	}
}

void GUI::closePOIFiles()
{
	_poiFilesMenu->clear();

	for (int i = 0; i < _poiFilesActions.count(); i++)
		delete _poiFilesActions[i];
	_poiFilesActions.clear();

	_poi->clear();
}

void GUI::printFile()
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog dialog(&printer, this);

	if (dialog.exec() == QDialog::Accepted)
		plot(&printer);
}

void GUI::exportFile()
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setCreator(QString(APP_NAME) + QString(" ") + QString(APP_VERSION));
	printer.setOrientation(_exportOrientation);
	printer.setOutputFileName(_exportFileName);
	printer.setPaperSize(_exportPaperSize);
	printer.setPageMargins(_exportMargins.left(), _exportMargins.top(),
	  _exportMargins.right(), _exportMargins.bottom(), QPrinter::Millimeter);
	ExportDialog dialog(&printer, this);

	if (dialog.exec() == QDialog::Accepted) {
		_exportFileName = printer.outputFileName();
		_exportPaperSize = printer.paperSize();
		_exportOrientation = printer.orientation();
		printer.getPageMargins(&(_exportMargins.rleft()),
		  &(_exportMargins.rtop()), &(_exportMargins.rright()),
		  &(_exportMargins.rbottom()), QPrinter::Millimeter);
		plot(&printer);
	}
}

void GUI::plot(QPrinter *printer)
{
	QPainter p(printer);
	TrackInfo info;
	qreal ih, gh, mh, ratio;
	Units units = _imperialUnitsAction->isChecked() ? Imperial : Metric;
	qreal d = distance();
	qreal t = time();

	if (_dateRange.first.isValid()) {
		if (_dateRange.first == _dateRange.second) {
			QString format = QLocale::system().dateFormat(QLocale::LongFormat);
			info.insert(tr("Date"), _dateRange.first.toString(format));
		} else {
			QString format = QLocale::system().dateFormat(QLocale::ShortFormat);
			info.insert(tr("Date"), QString("%1 - %2")
			  .arg(_dateRange.first.toString(format),
			  _dateRange.second.toString(format)));
		}
	}

	if (_trackCount > 1)
		info.insert(tr("Tracks"), QString::number(_trackCount));
	if (_routeCount > 1)
		info.insert(tr("Routes"), QString::number(_routeCount));
	if (_waypointCount > 2)
		info.insert(tr("Waypoints"), QString::number(_waypointCount));

	if (d > 0)
		info.insert(tr("Distance"), ::distance(d, units));
	if (t > 0)
		info.insert(tr("Time"), ::timeSpan(t));


	ratio = p.paintEngine()->paintDevice()->logicalDpiX() / SCREEN_DPI;
	if (info.isEmpty()) {
		ih = 0;
		mh = 0;
	} else {
		ih = info.contentSize().height() * ratio;
		mh = ih / 2;
		info.plot(&p, QRectF(0, 0, printer->width(), ih));
	}
	if (_graphTabWidget->isVisible()) {
		qreal r = (((qreal)(printer)->width()) / (qreal)(printer->height()));
		gh = (printer->width() > printer->height())
		  ? 0.15 * r * (printer->height() - ih - 2*mh)
		  : 0.15 * (printer->height() - ih - 2*mh);
		gh = qMax(gh, ratio * 150);
		GraphTab *gt = static_cast<GraphTab*>(_graphTabWidget->currentWidget());
		gt->plot(&p,  QRectF(0, printer->height() - gh, printer->width(), gh));
	} else
		gh = 0;
	_pathView->plot(&p, QRectF(0, ih + mh, printer->width(), printer->height()
	  - (ih + 2*mh + gh)));
}

void GUI::reloadFile()
{
	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_dateRange = DateRange(QDate(), QDate());

	for (int i = 0; i < _tabs.count(); i++)
		_tabs.at(i)->clear();
	_pathView->clear();

	_sliderPos = 0;

	for (int i = 0; i < _files.size(); i++) {
		if (!loadFile(_files.at(i))) {
			_files.removeAt(i);
			i--;
		}
	}

	updateStatusBarInfo();
	updateWindowTitle();
	updateGraphTabs();
	updateTrackView();
	if (_files.isEmpty())
		_fileActionGroup->setEnabled(false);
	else
		_browser->setCurrent(_files.last());
}

void GUI::closeFiles()
{
	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_dateRange = DateRange(QDate(), QDate());

	_sliderPos = 0;

	for (int i = 0; i < _tabs.count(); i++)
		_tabs.at(i)->clear();
	_pathView->clear();

	_files.clear();
}

void GUI::closeAll()
{
	closeFiles();

	_fileActionGroup->setEnabled(false);
	updateStatusBarInfo();
	updateWindowTitle();
	updateGraphTabs();
	updateTrackView();
}

void GUI::showMap(bool show)
{
	if (show)
		_pathView->setMap(_currentMap);
	else
		_pathView->setMap(0);
}

void GUI::showGraphs(bool show)
{
	_graphTabWidget->setHidden(!show);
}

void GUI::showToolbars(bool show)
{
	if (show) {
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

void GUI::showFullscreen(bool show)
{
	if (show) {
		_frameStyle = _pathView->frameStyle();
		_showGraphs = _showGraphsAction->isChecked();

		statusBar()->hide();
		menuBar()->hide();
		showToolbars(false);
		showGraphs(false);
		_showGraphsAction->setChecked(false);
		_pathView->setFrameStyle(QFrame::NoFrame);

		showFullScreen();
	} else {
		statusBar()->show();
		menuBar()->show();
		if (_showToolbarsAction->isChecked())
			showToolbars(true);
		_showGraphsAction->setChecked(_showGraphs);
		if (_showGraphsAction->isEnabled())
			showGraphs(_showGraphs);
		_pathView->setFrameStyle(_frameStyle);

		showNormal();
	}
}

void GUI::showTracks(bool show)
{
	_pathView->showTracks(show);

	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showTracks(show);

	updateStatusBarInfo();
}

void GUI::showRoutes(bool show)
{
	_pathView->showRoutes(show);

	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showRoutes(show);

	updateStatusBarInfo();
}

void GUI::showGraphGrids(bool show)
{
	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showGrid(show);
}

void GUI::clearMapCache()
{
	_currentMap->clearCache();
	_pathView->redraw();
}

void GUI::updateStatusBarInfo()
{
	if (_files.count() == 0)
		_fileNameLabel->setText(tr("No GPX files loaded"));
	else if (_files.count() == 1)
		_fileNameLabel->setText(_files.at(0));
	else
		_fileNameLabel->setText(tr("%1 files", "", _files.count())
		  .arg(_files.count()));

	qreal d = distance();
	Units units = _imperialUnitsAction->isChecked() ? Imperial : Metric;
	if (d > 0)
		_distanceLabel->setText(::distance(distance(), units));
	else
		_distanceLabel->clear();

	qreal t = time();
	if (t > 0)
		_timeLabel->setText(::timeSpan(time()));
	else
		_timeLabel->clear();
}

void GUI::updateWindowTitle()
{
	if (_files.count() == 1)
		setWindowTitle(QFileInfo(_files.at(0)).fileName()
		  + QString(" - " APP_NAME));
	else
		setWindowTitle(APP_NAME);
}

void GUI::mapChanged(int index)
{
	_currentMap = _maps.at(index);

	if (_showMapAction->isChecked())
		_pathView->setMap(_currentMap);
}

void GUI::nextMap()
{
	if (_maps.count() < 2)
		return;
	int next = (_maps.indexOf(_currentMap) + 1) % _maps.count();
	_mapActions.at(next)->setChecked(true);
	mapChanged(next);
}

void GUI::prevMap()
{
	if (_maps.count() < 2)
		return;
	int prev = (_maps.indexOf(_currentMap) + _maps.count() - 1) % _maps.count();
	_mapActions.at(prev)->setChecked(true);
	mapChanged(prev);
}

void GUI::poiFileChecked(int index)
{
	_poi->enableFile(_poi->files().at(index),
	  _poiFilesActions.at(index)->isChecked());
}

void GUI::sliderPositionChanged(qreal pos)
{
	_sliderPos = pos;
}

void GUI::graphChanged(int index)
{
	if (index < 0)
		return;

	GraphTab *gt = static_cast<GraphTab*>(_graphTabWidget->widget(index));
	gt->setSliderPosition(_sliderPos);
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
	int index;
	GraphTab *tab;

	for (int i = 0; i < _tabs.size(); i++) {
		tab = _tabs.at(i);
		if (!tab->count() && (index = _graphTabWidget->indexOf(tab)) >= 0)
			_graphTabWidget->removeTab(index);
	}

	for (int i = 0; i < _tabs.size(); i++) {
		tab = _tabs.at(i);
		if (tab->count() && _graphTabWidget->indexOf(tab) < 0)
			_graphTabWidget->insertTab(i, tab, _tabs.at(i)->label());
	}

	if (_graphTabWidget->count()) {
		if (_showGraphsAction->isChecked())
			_graphTabWidget->setHidden(false);
		_showGraphsAction->setEnabled(true);
	} else {
		_graphTabWidget->setHidden(true);
		_showGraphsAction->setEnabled(false);
	}
}

void GUI::updateTrackView()
{
	_pathView->setHidden(!(_pathView->trackCount() + _pathView->routeCount()
	  + _pathView->waypointCount()));
}

void GUI::setUnits(Units units)
{
	_pathView->setUnits(units);
	for (int i = 0; i <_tabs.count(); i++)
		_tabs.at(i)->setUnits(units);
	updateStatusBarInfo();
}

void GUI::setGraphType(GraphType type)
{
	_sliderPos = 0;

	for (int i = 0; i <_tabs.count(); i++) {
		_tabs.at(i)->setGraphType(type);
		_tabs.at(i)->setSliderPosition(0);
	}
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

		case Qt::Key_Escape:
			if (_fullscreenAction->isChecked()) {
				_fullscreenAction->setChecked(false);
				showFullscreen(false);
			}
			break;
	}

	if (!file.isNull()) {
		if (!(event->modifiers() & MODIFIER))
			closeFiles();
		openFile(file);
	}
}

void GUI::closeEvent(QCloseEvent *event)
{
	writeSettings();
	event->accept();
}

void GUI::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;
	if (event->proposedAction() != Qt::CopyAction)
		return;

	QList<QUrl> urls = event->mimeData()->urls();
	for (int i = 0; i < urls.size(); i++)
		if (!urls.at(i).isLocalFile())
			return;

	event->acceptProposedAction();
}

void GUI::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	for (int i = 0; i < urls.size(); i++)
		openFile(urls.at(i).toLocalFile());
}

void GUI::writeSettings()
{
	QSettings settings(APP_NAME, APP_NAME);

	settings.beginGroup(WINDOW_SETTINGS_GROUP);
	settings.setValue(WINDOW_SIZE_SETTING, size());
	settings.setValue(WINDOW_POS_SETTING, pos());
	settings.endGroup();

	settings.beginGroup(SETTINGS_SETTINGS_GROUP);
	settings.setValue(UNITS_SETTING, _imperialUnitsAction->isChecked()
	  ? Imperial : Metric);
	settings.setValue(SHOW_TOOLBARS_SETTING, _showToolbarsAction->isChecked());
	settings.endGroup();

	settings.beginGroup(MAP_SETTINGS_GROUP);
	if (_currentMap)
		settings.setValue(CURRENT_MAP_SETTING, _currentMap->name());
	settings.setValue(SHOW_MAP_SETTING, _showMapAction->isChecked());
	settings.endGroup();

	settings.beginGroup(GRAPH_SETTINGS_GROUP);
	settings.setValue(SHOW_GRAPHS_SETTING, _showGraphsAction->isChecked());
	settings.setValue(GRAPH_TYPE_SETTING, _timeGraphAction->isChecked()
	  ? Time : Distance);
	settings.setValue(SHOW_GRAPH_GRIDS_SETTING,
	  _showGraphGridAction->isChecked());
	settings.endGroup();

	settings.beginGroup(POI_SETTINGS_GROUP);
	settings.setValue(SHOW_POI_SETTING, _showPOIAction->isChecked());
	settings.setValue(OVERLAP_POI_SETTING, _overlapPOIAction->isChecked());

	settings.remove(DISABLED_POI_FILE_SETTINGS_PREFIX);
	settings.beginWriteArray(DISABLED_POI_FILE_SETTINGS_PREFIX);
	for (int i = 0, j = 0; i < _poiFilesActions.count(); i++) {
		if (!_poiFilesActions.at(i)->isChecked()) {
			settings.setArrayIndex(j++);
			settings.setValue(DISABLED_POI_FILE_SETTING, _poi->files().at(i));
		}
	}
	settings.endArray();
	settings.endGroup();

	settings.beginGroup(DATA_SETTINGS_GROUP);
	settings.setValue(SHOW_TRACKS_SETTING, _showTracksAction->isChecked());
	settings.setValue(SHOW_ROUTES_SETTING, _showRoutesAction->isChecked());
	settings.setValue(SHOW_WAYPOINTS_SETTING,
	  _showWaypointsAction->isChecked());
	settings.setValue(SHOW_WAYPOINT_LABELS_SETTING,
	  _showWaypointLabelsAction->isChecked());
	settings.setValue(SHOW_ROUTE_WAYPOINTS_SETTING,
	  _showRouteWaypointsAction->isChecked());

	settings.endGroup();
}

void GUI::readSettings()
{
	QSettings settings(APP_NAME, APP_NAME);

	settings.beginGroup(WINDOW_SETTINGS_GROUP);
	resize(settings.value(WINDOW_SIZE_SETTING, QSize(600, 800)).toSize());
	move(settings.value(WINDOW_POS_SETTING, QPoint(10, 10)).toPoint());
	settings.endGroup();

	settings.beginGroup(SETTINGS_SETTINGS_GROUP);
	Units u = QLocale::system().measurementSystem() == QLocale::ImperialSystem
	  ? Imperial : Metric;
	if (settings.value(UNITS_SETTING, u).toInt() == Imperial) {
		setImperialUnits();
		_imperialUnitsAction->setChecked(true);
	} else
		_metricUnitsAction->setChecked(true);
	if (settings.value(SHOW_TOOLBARS_SETTING, true).toBool() == false)
		showToolbars(false);
	else
		_showToolbarsAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(MAP_SETTINGS_GROUP);
	if (settings.value(SHOW_MAP_SETTING, true).toBool() == true)
		_showMapAction->setChecked(true);
	if (_maps.count()) {
		int index = mapIndex(settings.value(CURRENT_MAP_SETTING).toString());
		_mapActions.at(index)->setChecked(true);
		_currentMap = _maps.at(index);
		if (_showMapAction->isChecked())
			_pathView->setMap(_currentMap);
	} else
		_currentMap = 0;
	settings.endGroup();

	settings.beginGroup(GRAPH_SETTINGS_GROUP);
	if (settings.value(SHOW_GRAPHS_SETTING, true).toBool() == false)
		showGraphs(false);
	else
		_showGraphsAction->setChecked(true);
	if (settings.value(GRAPH_TYPE_SETTING, Distance).toInt() == Time) {
		setTimeGraph();
		_timeGraphAction->setChecked(true);
	} else
		_distanceGraphAction->setChecked(true);
	if (settings.value(SHOW_GRAPH_GRIDS_SETTING, true).toBool() == false)
		showGraphGrids(false);
	else
		_showGraphGridAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(POI_SETTINGS_GROUP);
	if (settings.value(OVERLAP_POI_SETTING, true).toBool() == false)
		_pathView->setPOIOverlap(false);
	else
		_overlapPOIAction->setChecked(true);
	if (settings.value(LABELS_POI_SETTING, true).toBool() == false)
		_pathView->showPOILabels(false);
	else
		_showPOILabelsAction->setChecked(true);
	if (settings.value(SHOW_POI_SETTING, false).toBool() == true)
		_showPOIAction->setChecked(true);
	else
		_pathView->showPOI(false);
	for (int i = 0; i < _poiFilesActions.count(); i++)
		_poiFilesActions.at(i)->setChecked(true);
	int size = settings.beginReadArray(DISABLED_POI_FILE_SETTINGS_PREFIX);
	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		int index = _poi->files().indexOf(settings.value(
		  DISABLED_POI_FILE_SETTING).toString());
		if (index >= 0) {
			_poi->enableFile(_poi->files().at(index), false);
			_poiFilesActions.at(index)->setChecked(false);
		}
	}
	settings.endArray();
	settings.endGroup();

	settings.beginGroup(DATA_SETTINGS_GROUP);
	if (settings.value(SHOW_TRACKS_SETTING, true).toBool() == false) {
		_pathView->showTracks(false);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showTracks(false);
	} else
		_showTracksAction->setChecked(true);
	if (settings.value(SHOW_ROUTES_SETTING, true).toBool() == false) {
		_pathView->showRoutes(false);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showRoutes(false);
	} else
		_showRoutesAction->setChecked(true);
	if (settings.value(SHOW_WAYPOINTS_SETTING, true).toBool() == false)
		_pathView->showWaypoints(false);
	else
		_showWaypointsAction->setChecked(true);
	if (settings.value(SHOW_WAYPOINT_LABELS_SETTING, true).toBool() == false)
		_pathView->showWaypointLabels(false);
	else
		_showWaypointLabelsAction->setChecked(true);
	if (settings.value(SHOW_ROUTE_WAYPOINTS_SETTING, true).toBool() == false)
		_pathView->showRouteWaypoints(false);
	else
		_showRouteWaypointsAction->setChecked(true);
	settings.endGroup();
}

int GUI::mapIndex(const QString &name)
{
	for (int i = 0; i < _maps.count(); i++)
		if (_maps.at(i)->name() == name)
			return i;

	return 0;
}

qreal GUI::distance()
{
	qreal dist = 0;

	if (_showTracksAction->isChecked())
		dist += _trackDistance;
	if (_showRoutesAction->isChecked())
		dist += _routeDistance;

	return dist;
}

qreal GUI::time()
{
	return (_showTracksAction->isChecked()) ? _time : 0;
}
