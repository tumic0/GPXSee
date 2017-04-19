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
#include "config.h"
#include "icons.h"
#include "keys.h"
#include "settings.h"
#include "data.h"
#include "ellipsoid.h"
#include "datum.h"
#include "map.h"
#include "maplist.h"
#include "mapdir.h"
#include "emptymap.h"
#include "elevationgraph.h"
#include "speedgraph.h"
#include "heartrategraph.h"
#include "temperaturegraph.h"
#include "cadencegraph.h"
#include "powergraph.h"
#include "pathview.h"
#include "trackinfo.h"
#include "filebrowser.h"
#include "cpuarch.h"
#include "graphtab.h"
#include "format.h"
#include "gui.h"


GUI::GUI()
{
	loadDatums();
	loadMaps();
	loadPOIs();

	createPathView();
	createGraphTabs();
	createStatusBar();
	createActions();
	createMenus();
	createToolBars();

	createBrowser();

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
	setAcceptDrops(true);

	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_movingTime = 0;

	_sliderPos = 0;

	updateGraphTabs();
	updatePathView();
	updateStatusBarInfo();

	readSettings();
}

GUI::~GUI()
{
	for (int i = 0; i < _tabs.size(); i++) {
		if (_graphTabWidget->indexOf(_tabs.at(i)) < 0)
			delete _tabs.at(i);
	}
}

const QString GUI::getLastUsedTrackDirectory()
{
	if (!_lastOpenDirectory.exists()) {
		_lastOpenDirectory = QDir(QDir::homePath());
	}
	return _lastOpenDirectory.absolutePath();
}

void GUI::setLastUsedTrackDirectory(const QString& directoryName)
{
	_lastOpenDirectory = QDir(QDir(directoryName).absolutePath());
}

const QString GUI::fileFormats() const
{
	return tr("Supported files (*.csv *.fit *.gpx *.igc *.kml *.nmea *.tcx)")
	  + ";;" + tr("CSV files (*.csv)") + ";;" + tr("FIT files (*.fit)") + ";;"
	  + tr("GPX files (*.gpx)") + ";;" + tr("IGC files (*.igc)") + ";;"
	  + tr("KML files (*.kml)") + ";;" + tr("NMEA files (*.nmea)") + ";;"
	  + tr("TCX files (*.tcx)") + ";;" + tr("All files (*)");
}

void GUI::createBrowser()
{
	QStringList filter;
	filter << "*.gpx" << "*.tcx" << "*.kml" << "*.fit" << "*.csv" << "*.igc"
	  << "*.nmea";
	_browser = new FileBrowser(this);
	_browser->setFilter(filter);
}

void GUI::loadDatums()
{
	QString ef, df;
	bool ok = false;

	if (QFile::exists(USER_ELLIPSOID_FILE))
		ef = USER_ELLIPSOID_FILE;
	else if (QFile::exists(GLOBAL_ELLIPSOID_FILE))
		ef = GLOBAL_ELLIPSOID_FILE;
	else
		qWarning("No ellipsoids file found.");

	if (QFile::exists(USER_DATUM_FILE))
		df = USER_DATUM_FILE;
	else if (QFile::exists(GLOBAL_DATUM_FILE))
		df = GLOBAL_DATUM_FILE;
	else
		qWarning("No datums file found.");

	if (!ef.isNull() && !df.isNull()) {
		if (!Ellipsoid::loadList(ef)) {
			if (Ellipsoid::errorLine())
				qWarning("%s: parse error on line %d: %s", qPrintable(ef),
				  Ellipsoid::errorLine(), qPrintable(Ellipsoid::errorString()));
			else
				qWarning("%s: %s", qPrintable(ef), qPrintable(
				  Ellipsoid::errorString()));
		} else {
			if (!Datum::loadList(df)) {
				if (Datum::errorLine())
					qWarning("%s: parse error on line %d: %s", qPrintable(ef),
					  Datum::errorLine(), qPrintable(Datum::errorString()));
				else
					qWarning("%s: %s", qPrintable(ef), qPrintable(
					  Datum::errorString()));
			} else
				ok = true;
		}
	}

	if (!ok)
		qWarning("Maps based on a datum different from WGS84 won't work.");
}

void GUI::loadMaps()
{
	QList<Map*> online, offline;

	if (QFile::exists(USER_MAP_FILE))
		online = MapList::load(USER_MAP_FILE, this);
	else
		online = MapList::load(GLOBAL_MAP_FILE, this);

	if (QFile::exists(USER_MAP_DIR))
		offline = MapDir::load(USER_MAP_DIR, this);
	else
		offline = MapDir::load(GLOBAL_MAP_DIR, this);

	_maps = online + offline;

	_map = _maps.isEmpty() ? new EmptyMap(this) : _maps.first();
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

	for (int i = 0; i < list.size(); ++i) {
		if (!_poi->loadFile(list.at(i).absoluteFilePath())) {
			qWarning("Error loading POI file: %s: %s\n",
			  qPrintable(list.at(i).fileName()),
			  qPrintable(_poi->errorString()));
			if (_poi->errorLine())
				qWarning("Line: %d\n", _poi->errorLine());
		}
	}
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
	connect(_showMapAction, SIGNAL(triggered(bool)), _pathView,
	  SLOT(showMap(bool)));
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
	_totalTimeAction = new QAction(tr("Total time"), this);
	_totalTimeAction->setCheckable(true);
	_totalTimeAction->setActionGroup(ag);
	connect(_totalTimeAction, SIGNAL(triggered()), this,
	  SLOT(setTotalTime()));
	_movingTimeAction = new QAction(tr("Moving time"), this);
	_movingTimeAction->setCheckable(true);
	_movingTimeAction->setActionGroup(ag);
	connect(_movingTimeAction, SIGNAL(triggered()), this,
	  SLOT(setMovingTime()));
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
	_openOptionsAction = new QAction(tr("Options..."), this);
	connect(_openOptionsAction, SIGNAL(triggered()), this,
	  SLOT(openOptions()));

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
	QMenu *timeMenu = settingsMenu->addMenu(tr("Time"));
	timeMenu->addAction(_totalTimeAction);
	timeMenu->addAction(_movingTimeAction);
	QMenu *unitsMenu = settingsMenu->addMenu(tr("Units"));
	unitsMenu->addAction(_metricUnitsAction);
	unitsMenu->addAction(_imperialUnitsAction);
	settingsMenu->addSeparator();
	settingsMenu->addAction(_showToolbarsAction);
	settingsMenu->addAction(_fullscreenAction);
	settingsMenu->addSeparator();
	settingsMenu->addAction(_openOptionsAction);

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
	_pathView = new PathView(_map, _poi, this);
	_pathView->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,
	  QSizePolicy::Expanding));
	_pathView->setMinimumHeight(200);
#ifdef Q_OS_WIN32
	_pathView->setFrameShape(QFrame::NoFrame);
#endif // Q_OS_WIN32
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
	_tabs.append(new CadenceGraph);
	_tabs.append(new PowerGraph);
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
	msgBox.setText("<h2>" + QString(APP_NAME) + "</h2><p><p>" + tr("Version ")
	  + APP_VERSION + " (" + CPU_ARCH + ", Qt " + QT_VERSION_STR + ")</p>");
	msgBox.setInformativeText("<table width=\"300\"><tr><td>"
	  + tr("GPXSee is distributed under the terms of the GNU General Public "
	  "License version 3. For more info about GPXSee visit the project "
	  "homepage at ") + "<a href=\"" + APP_HOMEPAGE + "\">" + APP_HOMEPAGE
	  + "</a>.</td></tr></table>");

	QIcon icon = msgBox.windowIcon();
	QSize size = icon.actualSize(QSize(64, 64));
	msgBox.setIconPixmap(icon.pixmap(size));

	msgBox.exec();
}

void GUI::keys()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("Keyboard controls"));
	msgBox.setText("<h3>" + tr("Keyboard controls") + "</h3>");
	msgBox.setInformativeText(
	  "<style>td {padding-right: 1.5em;}</style><div><table><tr><td>"
	  + tr("Next file") + "</td><td><i>" + QKeySequence(NEXT_KEY).toString()
	  + "</i></td></tr><tr><td>" + tr("Previous file")
	  + "</td><td><i>" + QKeySequence(PREV_KEY).toString()
	  + "</i></td></tr><tr><td>" + tr("First file") + "</td><td><i>"
	  + QKeySequence(FIRST_KEY).toString() + "</i></td></tr><tr><td>"
	  + tr("Last file") + "</td><td><i>" + QKeySequence(LAST_KEY).toString()
	  + "</i></td></tr><tr><td>" + tr("Append file")
	  + "</td><td><i>" + QKeySequence(MODIFIER).toString() + tr("Next/Previous")
	  + "</i></td></tr><tr><td></td><td></td></tr><tr><td>" + tr("Next map")
	  + "</td><td><i>" + NEXT_MAP_SHORTCUT.toString() + "</i></td></tr><tr><td>"
	  + tr("Previous map") + "</td><td><i>" + PREV_MAP_SHORTCUT.toString()
	  + "</i></td></tr><tr><td></td><td></td></tr><tr><td>" + tr("Zoom in")
	  + "</td><td><i>" + QKeySequence(ZOOM_IN).toString()
	  + "</i></td></tr><tr><td>" + tr("Zoom out") + "</td><td><i>"
	  + QKeySequence(ZOOM_OUT).toString() + "</i></td></tr><tr><td>"
	  + tr("Digital zoom") + "</td><td><i>" + QKeySequence(MODIFIER).toString()
	  + tr("Zoom") + "</i></td></tr></table></div>");

	msgBox.exec();
}

void GUI::dataSources()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("Data sources"));
	msgBox.setText("<h3>" + tr("Data sources") + "</h3>");
	msgBox.setInformativeText(
	  "<h4>" + tr("Online maps") + "</h4><p>"
	  + tr("Online map URLs are read on program startup from the "
		"following file:")
	  + "</p><p><code>" + USER_MAP_FILE + "</code></p><p>"
	  + tr("The file format is one map entry per line, consisting of the map "
		"name and tiles URL delimited by a TAB character. The tile X and Y "
		"coordinates are replaced with $x and $y in the URL and the zoom "
		"level is replaced with $z. An example map file could look like:")
	  + "</p><p><code>Map1	http://tile.server.com/map/$z/$x/$y.png"
		  "<br/>Map2	http://mapserver.org/map/$z-$x-$y</code></p>"

	  + "<h4>" + tr("Offline maps") + "</h4><p>"
	  + tr("Offline maps are loaded on program startup from the following "
		"directory:")
	  + "</p><p><code>" + USER_MAP_DIR + "</code></p><p>"
	  + tr("The expected structure is one map/atlas in a separate subdirectory."
		" Supported map formats are OziExplorer image-based maps and tiled"
		" TrekBuddy maps/atlases (tared and non-tared).") + "</p>"

	  + "<h4>" + tr("POIs") + "</h4><p>"
	  + tr("To make GPXSee load a POI file automatically on startup, add "
		"the file to the following directory:")
		+ "</p><p><code>" + USER_POI_DIR + "</code></p>"
	);

	msgBox.exec();
}

void GUI::openFile()
{
	QStringList files = QFileDialog::getOpenFileNames(this,
													  tr("Open file"),
													  getLastUsedTrackDirectory(),
													  fileFormats());
	QStringList list = files;

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		openFile(*it);

	if (!list.empty()) {
		setLastUsedTrackDirectory(QFileInfo(list.first()).absolutePath());
	}
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

		updateNavigationActions();
		updateStatusBarInfo();
		updateWindowTitle();
		updateGraphTabs();
		updatePathView();
	} else {
		if (_files.isEmpty())
			_fileActionGroup->setEnabled(false);
		ret = false;
	}

	return ret;
}

bool GUI::loadFile(const QString &fileName)
{
	Data data;
	QList<PathItem*> paths;

	if (data.loadFile(fileName)) {
		paths = _pathView->loadData(data);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->loadData(data, paths);

		for (int i = 0; i < data.tracks().count(); i++) {
			_trackDistance += data.tracks().at(i)->distance();
			_time += data.tracks().at(i)->time();
			_movingTime += data.tracks().at(i)->movingTime();
			const QDate &date = data.tracks().at(i)->date().date();
			if (_dateRange.first.isNull() || _dateRange.first > date)
				_dateRange.first = date;
			if (_dateRange.second.isNull() || _dateRange.second < date)
				_dateRange.second = date;
		}
		_trackCount += data.tracks().count();

		for (int i = 0; i < data.routes().count(); i++)
			_routeDistance += data.routes().at(i)->distance();
		_routeCount += data.routes().count();

		_waypointCount += data.waypoints().count();

		if (_pathName.isNull()) {
			if (data.tracks().count() == 1 && !data.routes().count())
				_pathName = data.tracks().first()->name();
			else if (data.routes().count() == 1 && !data.tracks().count())
				_pathName = data.routes().first()->name();
		} else
			_pathName = QString();

		return true;
	} else {
		updateNavigationActions();
		updateStatusBarInfo();
		updateWindowTitle();
		updateGraphTabs();
		updatePathView();

		QString error = tr("Error loading data file:") + "\n\n"
		  + fileName + "\n\n" + data.errorString();
		if (data.errorLine())
			error.append("\n" + tr("Line: %1").arg(data.errorLine()));
		QMessageBox::critical(this, APP_NAME, error);
		return false;
	}
}

void GUI::openPOIFile()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Open POI file"),
	  QString(), fileFormats());
	QStringList list = files;

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		openPOIFile(*it);
}

bool GUI::openPOIFile(const QString &fileName)
{
	if (fileName.isEmpty() || _poi->files().contains(fileName))
		return false;

	if (!_poi->loadFile(fileName)) {
		QString error = tr("Error loading POI file:") + "\n\n"
		  + fileName + "\n\n" + _poi->errorString();
		if (_poi->errorLine())
			error.append("\n" + tr("Line: %1").arg(_poi->errorLine()));
		QMessageBox::critical(this, APP_NAME, error);

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

void GUI::openOptions()
{
	Options options(_options);

	OptionsDialog dialog(&options, this);
	if (dialog.exec() != QDialog::Accepted)
		return;

	if (options.palette != _options.palette) {
		_pathView->setPalette(options.palette);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->setPalette(options.palette);
	}
	if (options.trackWidth != _options.trackWidth)
		_pathView->setTrackWidth(options.trackWidth);
	if (options.routeWidth != _options.routeWidth)
		_pathView->setRouteWidth(options.routeWidth);
	if (options.trackStyle != _options.trackStyle)
		_pathView->setTrackStyle(options.trackStyle);
	if (options.routeStyle != _options.routeStyle)
		_pathView->setRouteStyle(options.routeStyle);
	if (options.pathAntiAliasing != _options.pathAntiAliasing)
		_pathView->setRenderHint(QPainter::Antialiasing,
		  options.pathAntiAliasing);
	if (options.graphWidth != _options.graphWidth)
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->setGraphWidth(options.graphWidth);
	if (options.graphAntiAliasing != _options.graphAntiAliasing)
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->setRenderHint(QPainter::Antialiasing,
			  options.graphAntiAliasing);

	if (options.poiRadius != _options.poiRadius)
		_poi->setRadius(options.poiRadius);

	if (options.useOpenGL != _options.useOpenGL) {
		_pathView->useOpenGL(options.useOpenGL);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->useOpenGL(options.useOpenGL);
	}

	_options = options;
}

void GUI::exportFile()
{
	ExportDialog dialog(&_export, this);
	if (dialog.exec() != QDialog::Accepted)
		return;

	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setCreator(QString(APP_NAME) + QString(" ")
	  + QString(APP_VERSION));
	printer.setOrientation(_export.orientation);
	printer.setOutputFileName(_export.fileName);
	printer.setPaperSize(_export.paperSize);
	printer.setPageMargins(_export.margins.left(), _export.margins.top(),
	  _export.margins.right(), _export.margins.bottom(), QPrinter::Millimeter);

	plot(&printer);
}

void GUI::plot(QPrinter *printer)
{
	QPainter p(printer);
	TrackInfo info;
	qreal ih, gh, mh, ratio;
	qreal d = distance();
	qreal t = time();
	qreal tm = movingTime();

	if (!_pathName.isNull() && _options.printName)
		info.insert(tr("Name"), _pathName);

	if (_options.printItemCount) {
		if (_trackCount > 1)
			info.insert(tr("Tracks"), QString::number(_trackCount));
		if (_routeCount > 1)
			info.insert(tr("Routes"), QString::number(_routeCount));
		if (_waypointCount > 2)
			info.insert(tr("Waypoints"), QString::number(_waypointCount));
	}

	if (_dateRange.first.isValid() && _options.printDate) {
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

	if (d > 0 && _options.printDistance)
		info.insert(tr("Distance"), Format::distance(d, units()));
	if (t > 0 && _options.printTime)
		info.insert(tr("Time"), Format::timeSpan(t));
	if (tm > 0 && _options.printMovingTime)
		info.insert(tr("Moving time"), Format::timeSpan(tm));


	ratio = p.paintEngine()->paintDevice()->logicalDpiX() / SCREEN_DPI;
	if (info.isEmpty()) {
		ih = 0;
		mh = 0;
	} else {
		ih = info.contentSize().height() * ratio;
		mh = ih / 2;
		info.plot(&p, QRectF(0, 0, printer->width(), ih));
	}
	if (_graphTabWidget->isVisible() && !_options.separateGraphPage) {
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

	if (_graphTabWidget->isVisible() && _options.separateGraphPage) {
		printer->newPage();

		int cnt = 0;
		for (int i = 0; i < _tabs.size(); i++)
			if (_tabs.at(i)->count())
				cnt++;

		qreal sp = ratio * 20;
		gh = qMin((printer->height() - ((cnt - 1) * sp))/(qreal)cnt,
		  0.20 * printer->height());

		qreal y = 0;
		for (int i = 0; i < _tabs.size(); i++) {
			if (_tabs.at(i)->count()) {
				_tabs.at(i)->plot(&p,  QRectF(0, y, printer->width(), gh));
				y += gh + sp;
			}
		}
	}
}

void GUI::reloadFile()
{
	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_movingTime = 0;
	_dateRange = DateRange(QDate(), QDate());
	_pathName = QString();

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
	updatePathView();
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
	_movingTime = 0;
	_dateRange = DateRange(QDate(), QDate());
	_pathName = QString();

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
	updatePathView();
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
	_map->clearCache();
	_pathView->redraw();
}

void GUI::updateStatusBarInfo()
{
	if (_files.count() == 0)
		_fileNameLabel->setText(tr("No files loaded"));
	else if (_files.count() == 1)
		_fileNameLabel->setText(_files.at(0));
	else
		_fileNameLabel->setText(tr("%n files", "", _files.count()));

	if (distance() > 0)
		_distanceLabel->setText(Format::distance(distance(), units()));
	else
		_distanceLabel->clear();

	if (time() > 0) {
		if (_movingTimeAction->isChecked()) {
			_timeLabel->setText(Format::timeSpan(movingTime())
			  + "<sub>M</sub>");
			_timeLabel->setToolTip(Format::timeSpan(time()));
		} else {
			_timeLabel->setText(Format::timeSpan(time()));
			_timeLabel->setToolTip(Format::timeSpan(movingTime())
			  + "<sub>M</sub>");
		}
	} else {
		_timeLabel->clear();
		_timeLabel->setToolTip(QString());
	}
}

void GUI::updateWindowTitle()
{
	if (_files.count() == 1)
		setWindowTitle(QFileInfo(_files.at(0)).fileName() + " - " + APP_NAME);
	else
		setWindowTitle(APP_NAME);
}

void GUI::mapChanged(int index)
{
	_map = _maps.at(index);
	_pathView->setMap(_map);
}

void GUI::nextMap()
{
	if (_maps.count() < 2)
		return;

	int next = (_maps.indexOf(_map) + 1) % _maps.count();
	_mapActions.at(next)->setChecked(true);
	mapChanged(next);
}

void GUI::prevMap()
{
	if (_maps.count() < 2)
		return;

	int prev = (_maps.indexOf(_map) + _maps.count() - 1) % _maps.count();
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

void GUI::updatePathView()
{
	_pathView->setHidden(!(_pathView->trackCount() + _pathView->routeCount()
	  + _pathView->waypointCount()));
}

void GUI::setTimeType(TimeType type)
{
	for (int i = 0; i <_tabs.count(); i++)
		_tabs.at(i)->setTimeType(type);

	updateStatusBarInfo();
}

void GUI::setUnits(Units units)
{
	_export.units = units;
	_options.units = units;

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
				return;
			}
			break;
	}

	if (!file.isNull()) {
		if (!(event->modifiers() & MODIFIER))
			closeFiles();
		openFile(file);
		return;
	}

	QMainWindow::keyPressEvent(event);
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
	settings.clear();

	settings.beginGroup(WINDOW_SETTINGS_GROUP);
	if (size() != WINDOW_SIZE_DEFAULT)
		settings.setValue(WINDOW_SIZE_SETTING, size());
	if (pos() != WINDOW_POS_DEFAULT)
		settings.setValue(WINDOW_POS_SETTING, pos());
	settings.endGroup();

	settings.beginGroup(SETTINGS_SETTINGS_GROUP);
	if ((_movingTimeAction->isChecked() ? Moving : Total) !=
	  TIME_TYPE_DEFAULT)
		settings.setValue(TIME_TYPE_SETTING, _movingTimeAction->isChecked()
	  ? Moving : Total);
	if ((_imperialUnitsAction->isChecked() ? Imperial : Metric) !=
	  UNITS_DEFAULT)
		settings.setValue(UNITS_SETTING, _imperialUnitsAction->isChecked()
	  ? Imperial : Metric);
	if (_showToolbarsAction->isChecked() != SHOW_TOOLBARS_DEFAULT)
		settings.setValue(SHOW_TOOLBARS_SETTING,
		  _showToolbarsAction->isChecked());
	settings.endGroup();

	settings.beginGroup(MAP_SETTINGS_GROUP);
	settings.setValue(CURRENT_MAP_SETTING, _map->name());
	if (_showMapAction->isChecked() != SHOW_MAP_DEFAULT)
		settings.setValue(SHOW_MAP_SETTING, _showMapAction->isChecked());
	settings.endGroup();

	settings.beginGroup(GRAPH_SETTINGS_GROUP);
	if (_showGraphsAction->isChecked() != SHOW_GRAPHS_DEFAULT)
		settings.setValue(SHOW_GRAPHS_SETTING, _showGraphsAction->isChecked());
	if ((_timeGraphAction->isChecked() ? Time : Distance) != GRAPH_TYPE_DEFAULT)
		settings.setValue(GRAPH_TYPE_SETTING, _timeGraphAction->isChecked()
		  ? Time : Distance);
	if (_showGraphGridAction->isChecked() != SHOW_GRAPH_GRIDS_DEFAULT)
		settings.setValue(SHOW_GRAPH_GRIDS_SETTING,
		  _showGraphGridAction->isChecked());
	settings.endGroup();

	settings.beginGroup(POI_SETTINGS_GROUP);
	if (_showPOIAction->isChecked() != SHOW_POI_DEFAULT)
		settings.setValue(SHOW_POI_SETTING, _showPOIAction->isChecked());
	if (_overlapPOIAction->isChecked() != OVERLAP_POI_DEFAULT)
		settings.setValue(OVERLAP_POI_SETTING, _overlapPOIAction->isChecked());

	int j = 0;
	for (int i = 0; i < _poiFilesActions.count(); i++) {
		if (!_poiFilesActions.at(i)->isChecked()) {
			if (j == 0)
				settings.beginWriteArray(DISABLED_POI_FILE_SETTINGS_PREFIX);
			settings.setArrayIndex(j++);
			settings.setValue(DISABLED_POI_FILE_SETTING, _poi->files().at(i));
		}
	}
	if (j != 0)
		settings.endArray();
	settings.endGroup();

	settings.beginGroup(DATA_SETTINGS_GROUP);
	if (_showTracksAction->isChecked() != SHOW_TRACKS_DEFAULT)
		settings.setValue(SHOW_TRACKS_SETTING, _showTracksAction->isChecked());
	if (_showRoutesAction->isChecked() != SHOW_ROUTES_DEFAULT)
		settings.setValue(SHOW_ROUTES_SETTING, _showRoutesAction->isChecked());
	if (_showWaypointsAction->isChecked() != SHOW_WAYPOINTS_DEFAULT)
		settings.setValue(SHOW_WAYPOINTS_SETTING,
		  _showWaypointsAction->isChecked());
	if (_showWaypointLabelsAction->isChecked() != SHOW_WAYPOINT_LABELS_DEFAULT)
		settings.setValue(SHOW_WAYPOINT_LABELS_SETTING,
		  _showWaypointLabelsAction->isChecked());
	if (_showRouteWaypointsAction->isChecked() != SHOW_ROUTE_WAYPOINTS_DEFAULT)
		settings.setValue(SHOW_ROUTE_WAYPOINTS_SETTING,
		  _showRouteWaypointsAction->isChecked());
	settings.endGroup();

	settings.beginGroup(EXPORT_SETTINGS_GROUP);
	if (_export.orientation != PAPER_ORIENTATION_DEFAULT)
		settings.setValue(PAPER_ORIENTATION_SETTING, _export.orientation);
	if (_export.paperSize != PAPER_SIZE_DEFAULT)
		settings.setValue(PAPER_SIZE_SETTING, _export.paperSize);
	if (_export.margins.left() != MARGIN_LEFT_DEFAULT)
		settings.setValue(MARGIN_LEFT_SETTING, _export.margins.left());
	if (_export.margins.top() != MARGIN_TOP_DEFAULT)
		settings.setValue(MARGIN_TOP_SETTING, _export.margins.top());
	if (_export.margins.right() != MARGIN_RIGHT_DEFAULT)
		settings.setValue(MARGIN_RIGHT_SETTING, _export.margins.right());
	if (_export.margins.bottom() != MARGIN_BOTTOM_DEFAULT)
		settings.setValue(MARGIN_BOTTOM_SETTING, _export.margins.bottom());
	if (_export.fileName != EXPORT_FILENAME_DEFAULT)
		settings.setValue(EXPORT_FILENAME_SETTING, _export.fileName);
	settings.endGroup();

	settings.beginGroup(OPTIONS_SETTINGS_GROUP);
	if (_options.palette.color() != PALETTE_COLOR_DEFAULT)
		settings.setValue(PALETTE_COLOR_SETTING, _options.palette.color());
	if (_options.palette.shift() != PALETTE_SHIFT_DEFAULT)
		settings.setValue(PALETTE_SHIFT_SETTING, _options.palette.shift());
	if (_options.trackWidth != TRACK_WIDTH_DEFAULT)
		settings.setValue(TRACK_WIDTH_SETTING, _options.trackWidth);
	if (_options.routeWidth != ROUTE_WIDTH_DEFAULT)
		settings.setValue(ROUTE_WIDTH_SETTING, _options.routeWidth);
	if (_options.trackStyle != TRACK_STYLE_DEFAULT)
		settings.setValue(TRACK_STYLE_SETTING, (int)_options.trackStyle);
	if (_options.routeStyle != ROUTE_STYLE_DEFAULT)
		settings.setValue(ROUTE_STYLE_SETTING, (int)_options.routeStyle);
	if (_options.graphWidth != GRAPH_WIDTH_DEFAULT)
		settings.setValue(GRAPH_WIDTH_SETTING, _options.graphWidth);
	if (_options.pathAntiAliasing != PATH_AA_DEFAULT)
		settings.setValue(PATH_AA_SETTING, _options.pathAntiAliasing);
	if (_options.graphAntiAliasing != GRAPH_AA_DEFAULT)
		settings.setValue(GRAPH_AA_SETTING, _options.graphAntiAliasing);
	if (_options.poiRadius != POI_RADIUS_DEFAULT)
		settings.setValue(POI_RADIUS_SETTING, _options.poiRadius);
	if (_options.useOpenGL != USE_OPENGL_DEFAULT)
		settings.setValue(USE_OPENGL_SETTING, _options.useOpenGL);
	if (_options.printName != PRINT_NAME_DEFAULT)
		settings.setValue(PRINT_NAME_SETTING, _options.printName);
	if (_options.printDate != PRINT_DATE_DEFAULT)
		settings.setValue(PRINT_DATE_SETTING, _options.printDate);
	if (_options.printDistance != PRINT_DISTANCE_DEFAULT)
		settings.setValue(PRINT_DISTANCE_SETTING, _options.printDistance);
	if (_options.printTime != PRINT_TIME_DEFAULT)
		settings.setValue(PRINT_TIME_SETTING, _options.printTime);
	if (_options.printMovingTime != PRINT_MOVING_TIME_DEFAULT)
		settings.setValue(PRINT_MOVING_TIME_SETTING, _options.printMovingTime);
	if (_options.printItemCount != PRINT_ITEM_COUNT_DEFAULT)
		settings.setValue(PRINT_ITEM_COUNT_SETTING, _options.printItemCount);
	if (_options.separateGraphPage != SEPARATE_GRAPH_PAGE_DEFAULT)
		settings.setValue(SEPARATE_GRAPH_PAGE_SETTING,
		  _options.separateGraphPage);
	settings.endGroup();

	settings.beginGroup(PRIVATE_SETTINGS_GROUP);
	if (getLastUsedTrackDirectory() != LAST_USED_DIRECTORY_DEFAULT)
		settings.setValue(LAST_USED_DIRECTORY_SETTING, getLastUsedTrackDirectory());
	settings.endGroup();
}

void GUI::readSettings()
{
	QSettings settings(APP_NAME, APP_NAME);

	settings.beginGroup(WINDOW_SETTINGS_GROUP);
	resize(settings.value(WINDOW_SIZE_SETTING, WINDOW_SIZE_DEFAULT).toSize());
	move(settings.value(WINDOW_POS_SETTING, WINDOW_POS_DEFAULT).toPoint());
	settings.endGroup();

	settings.beginGroup(SETTINGS_SETTINGS_GROUP);
	if (settings.value(TIME_TYPE_SETTING, TIME_TYPE_DEFAULT).toInt()
	  == Moving) {
		setTimeType(Moving);
		_movingTimeAction->setChecked(true);
	} else {
		setTimeType(Total);
		_totalTimeAction->setChecked(true);
	}
	if (settings.value(UNITS_SETTING, UNITS_DEFAULT).toInt() == Imperial) {
		setUnits(Imperial);
		_imperialUnitsAction->setChecked(true);
	} else {
		setUnits(Metric);
		_metricUnitsAction->setChecked(true);
	}
	if (!settings.value(SHOW_TOOLBARS_SETTING, SHOW_TOOLBARS_DEFAULT).toBool())
		showToolbars(false);
	else
		_showToolbarsAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(MAP_SETTINGS_GROUP);
	if (settings.value(SHOW_MAP_SETTING, SHOW_MAP_DEFAULT).toBool())
		_showMapAction->setChecked(true);
	if (_maps.count()) {
		int index = mapIndex(settings.value(CURRENT_MAP_SETTING).toString());
		_mapActions.at(index)->setChecked(true);
		_map = _maps.at(index);
		_pathView->setMap(_map);
	}
	settings.endGroup();

	settings.beginGroup(GRAPH_SETTINGS_GROUP);
	if (!settings.value(SHOW_GRAPHS_SETTING, SHOW_GRAPHS_DEFAULT).toBool())
		showGraphs(false);
	else
		_showGraphsAction->setChecked(true);
	if (settings.value(GRAPH_TYPE_SETTING, GRAPH_TYPE_DEFAULT).toInt()
	  == Time) {
		setTimeGraph();
		_timeGraphAction->setChecked(true);
	} else
		_distanceGraphAction->setChecked(true);
	if (!settings.value(SHOW_GRAPH_GRIDS_SETTING, SHOW_GRAPH_GRIDS_DEFAULT)
	  .toBool())
		showGraphGrids(false);
	else
		_showGraphGridAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(POI_SETTINGS_GROUP);
	if (!settings.value(OVERLAP_POI_SETTING, OVERLAP_POI_DEFAULT).toBool())
		_pathView->setPOIOverlap(false);
	else
		_overlapPOIAction->setChecked(true);
	if (!settings.value(LABELS_POI_SETTING, LABELS_POI_DEFAULT).toBool())
		_pathView->showPOILabels(false);
	else
		_showPOILabelsAction->setChecked(true);
	if (settings.value(SHOW_POI_SETTING, SHOW_POI_DEFAULT).toBool())
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
	if (!settings.value(SHOW_TRACKS_SETTING, SHOW_TRACKS_DEFAULT).toBool()) {
		_pathView->showTracks(false);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showTracks(false);
	} else
		_showTracksAction->setChecked(true);
	if (!settings.value(SHOW_ROUTES_SETTING, SHOW_ROUTES_DEFAULT).toBool()) {
		_pathView->showRoutes(false);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showRoutes(false);
	} else
		_showRoutesAction->setChecked(true);
	if (!settings.value(SHOW_WAYPOINTS_SETTING, SHOW_WAYPOINTS_DEFAULT)
	  .toBool())
		_pathView->showWaypoints(false);
	else
		_showWaypointsAction->setChecked(true);
	if (!settings.value(SHOW_WAYPOINT_LABELS_SETTING,
	  SHOW_WAYPOINT_LABELS_DEFAULT).toBool())
		_pathView->showWaypointLabels(false);
	else
		_showWaypointLabelsAction->setChecked(true);
	if (!settings.value(SHOW_ROUTE_WAYPOINTS_SETTING,
	  SHOW_ROUTE_WAYPOINTS_SETTING).toBool())
		_pathView->showRouteWaypoints(false);
	else
		_showRouteWaypointsAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(EXPORT_SETTINGS_GROUP);
	_export.orientation = (QPrinter::Orientation) settings.value(
	  PAPER_ORIENTATION_SETTING, PAPER_ORIENTATION_DEFAULT).toInt();
	_export.paperSize = (QPrinter::PaperSize) settings.value(PAPER_SIZE_SETTING,
	  PAPER_SIZE_DEFAULT).toInt();
	qreal ml = settings.value(MARGIN_LEFT_SETTING, MARGIN_LEFT_DEFAULT)
	  .toReal();
	qreal mt = settings.value(MARGIN_TOP_SETTING, MARGIN_TOP_DEFAULT).toReal();
	qreal mr = settings.value(MARGIN_RIGHT_SETTING, MARGIN_RIGHT_DEFAULT)
	  .toReal();
	qreal mb = settings.value(MARGIN_BOTTOM_SETTING, MARGIN_BOTTOM_DEFAULT)
	  .toReal();
	_export.margins = MarginsF(ml, mt, mr, mb);
	_export.fileName = settings.value(EXPORT_FILENAME_SETTING,
	 EXPORT_FILENAME_DEFAULT).toString();
	settings.endGroup();

	settings.beginGroup(OPTIONS_SETTINGS_GROUP);
	QColor pc = settings.value(PALETTE_COLOR_SETTING, PALETTE_COLOR_DEFAULT)
	  .value<QColor>();
	qreal ps = settings.value(PALETTE_SHIFT_SETTING, PALETTE_SHIFT_DEFAULT)
	  .toDouble();
	_options.palette = Palette(pc, ps);
	_options.trackWidth = settings.value(TRACK_WIDTH_SETTING,
	  TRACK_WIDTH_DEFAULT).toInt();
	_options.routeWidth = settings.value(ROUTE_WIDTH_SETTING,
	  ROUTE_WIDTH_DEFAULT).toInt();
	_options.trackStyle = (Qt::PenStyle) settings.value(TRACK_STYLE_SETTING,
	  (int)TRACK_STYLE_DEFAULT).toInt();
	_options.routeStyle = (Qt::PenStyle) settings.value(ROUTE_STYLE_SETTING,
	  (int)ROUTE_STYLE_DEFAULT).toInt();
	_options.pathAntiAliasing = settings.value(PATH_AA_SETTING, PATH_AA_DEFAULT)
	  .toBool();
	_options.graphWidth = settings.value(GRAPH_WIDTH_SETTING,
	  GRAPH_WIDTH_DEFAULT).toInt();
	_options.graphAntiAliasing = settings.value(GRAPH_AA_SETTING,
	  GRAPH_AA_DEFAULT).toBool();
	_options.poiRadius = settings.value(POI_RADIUS_SETTING, POI_RADIUS_DEFAULT)
	  .toInt();
	_options.useOpenGL = settings.value(USE_OPENGL_SETTING, USE_OPENGL_DEFAULT)
	  .toBool();
	_options.printName = settings.value(PRINT_NAME_SETTING, PRINT_NAME_DEFAULT)
	  .toBool();
	_options.printDate = settings.value(PRINT_DATE_SETTING, PRINT_DATE_DEFAULT)
	  .toBool();
	_options.printDistance = settings.value(PRINT_DISTANCE_SETTING,
	  PRINT_DISTANCE_DEFAULT).toBool();
	_options.printTime = settings.value(PRINT_TIME_SETTING, PRINT_TIME_DEFAULT)
	  .toBool();
	_options.printMovingTime = settings.value(PRINT_MOVING_TIME_SETTING,
	  PRINT_MOVING_TIME_DEFAULT).toBool();
	_options.printItemCount = settings.value(PRINT_ITEM_COUNT_SETTING,
	  PRINT_ITEM_COUNT_DEFAULT).toBool();
	_options.separateGraphPage = settings.value(SEPARATE_GRAPH_PAGE_SETTING,
	  SEPARATE_GRAPH_PAGE_DEFAULT).toBool();

	_pathView->setPalette(_options.palette);
	_pathView->setTrackWidth(_options.trackWidth);
	_pathView->setRouteWidth(_options.routeWidth);
	_pathView->setTrackStyle(_options.trackStyle);
	_pathView->setRouteStyle(_options.routeStyle);
	_pathView->setRenderHint(QPainter::Antialiasing, _options.pathAntiAliasing);
	if (_options.useOpenGL)
		_pathView->useOpenGL(true);

	for (int i = 0; i < _tabs.count(); i++) {
		_tabs.at(i)->setPalette(_options.palette);
		_tabs.at(i)->setGraphWidth(_options.graphWidth);
		_tabs.at(i)->setRenderHint(QPainter::Antialiasing,
		  _options.graphAntiAliasing);
		if (_options.useOpenGL)
			_tabs.at(i)->useOpenGL(true);
	}

	_poi->setRadius(_options.poiRadius);
	settings.endGroup();

	settings.beginGroup(PRIVATE_SETTINGS_GROUP);
	setLastUsedTrackDirectory(settings.value(LAST_USED_DIRECTORY_SETTING,
	  LAST_USED_DIRECTORY_DEFAULT).toString());
	settings.endGroup();
}

int GUI::mapIndex(const QString &name)
{
	for (int i = 0; i < _maps.count(); i++)
		if (_maps.at(i)->name() == name)
			return i;

	return 0;
}

Units GUI::units() const
{
	return _imperialUnitsAction->isChecked() ? Imperial : Metric;
}

qreal GUI::distance() const
{
	qreal dist = 0;

	if (_showTracksAction->isChecked())
		dist += _trackDistance;
	if (_showRoutesAction->isChecked())
		dist += _routeDistance;

	return dist;
}

qreal GUI::time() const
{
	return (_showTracksAction->isChecked()) ? _time : 0;
}

qreal GUI::movingTime() const
{
	return (_showTracksAction->isChecked()) ? _movingTime : 0;
}
