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
#include <QWindow>
#include <QScreen>
#include <QStyle>
#include <QTabBar>
#include "common/programpaths.h"
#include "data/data.h"
#include "data/poi.h"
#include "map/maplist.h"
#include "map/emptymap.h"
#include "map/downloader.h"
#include "map/crs.h"
#include "icons.h"
#include "keys.h"
#include "settings.h"
#include "elevationgraph.h"
#include "speedgraph.h"
#include "heartrategraph.h"
#include "temperaturegraph.h"
#include "cadencegraph.h"
#include "powergraph.h"
#include "gearratiograph.h"
#include "mapview.h"
#include "trackinfo.h"
#include "filebrowser.h"
#include "graphtab.h"
#include "graphitem.h"
#include "pathitem.h"
#include "mapitem.h"
#include "mapaction.h"
#include "poiaction.h"
#include "gui.h"


#define TOOLBAR_ICON_SIZE 22

GUI::GUI()
{
	TreeNode<MapAction*> mapActions;
	TreeNode<POIAction*> poiActions;

	_poi = new POI(this);

	createMapView();
	createGraphTabs();
	createStatusBar();
	createActions(mapActions, poiActions);
	createMenus(mapActions, poiActions);
	createToolBars();
	createBrowser();

	_splitter = new QSplitter();
	_splitter->setOrientation(Qt::Vertical);
	_splitter->setChildrenCollapsible(false);
	_splitter->addWidget(_mapView);
	_splitter->addWidget(_graphTabWidget);
	_splitter->setContentsMargins(0, 0, 0, 0);
	_splitter->setStretchFactor(0, 255);
	_splitter->setStretchFactor(1, 1);
	setCentralWidget(_splitter);

	setWindowIcon(QIcon(APP_ICON));
	setWindowTitle(APP_NAME);
	setUnifiedTitleAndToolBarOnMac(true);
	setAcceptDrops(true);

	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_areaCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_movingTime = 0;
	_lastTab = 0;

	readSettings();

	_dataDir = _options.dataPath;
	_mapDir = _options.mapsPath;
	_poiDir = _options.poiPath;

	updateGraphTabs();
	updateStatusBarInfo();
}

void GUI::createBrowser()
{
	_browser = new FileBrowser(this);
	_browser->setFilter(Data::filter());
	connect(_browser, &FileBrowser::listChanged, this,
	  &GUI::updateNavigationActions);
}

TreeNode<MapAction*> GUI::createMapActions()
{
	_mapsActionGroup = new QActionGroup(this);
	_mapsActionGroup->setExclusive(true);
	connect(_mapsActionGroup, &QActionGroup::triggered, this, &GUI::mapChanged);

	QString mapDir(ProgramPaths::mapDir());
	if (mapDir.isNull())
		return TreeNode<MapAction*>();

	TreeNode<Map*> maps(MapList::loadMaps(mapDir,
	  CRS::projection(_options.inputProjection)));
	return createMapActionsNode(maps);
}

TreeNode<MapAction*> GUI::createMapActionsNode(const TreeNode<Map*> &node)
{
	TreeNode<MapAction*> tree(node.name());

	for (int i = 0; i < node.childs().size(); i++)
		tree.addChild(createMapActionsNode(node.childs().at(i)));

	for (int i = 0; i < node.items().size(); i++) {
		Map *map = node.items().at(i);
		if (map->isValid()) {
			MapAction *a = new MapAction(map, _mapsActionGroup);
			connect(a, &MapAction::loaded, this, &GUI::mapInitialized);
			tree.addItem(a);
		} else {
			qWarning("%s: %s", qPrintable(map->path()),
			  qPrintable(map->errorString()));
			delete map;
		}
	}

	return tree;
}

void GUI::mapInitialized()
{
	MapAction *action = static_cast<MapAction*>(QObject::sender());
	Map *map = action->data().value<Map*>();

	if (map->isValid()) {
		if (!_mapsActionGroup->checkedAction())
			action->trigger();
		_showMapAction->setEnabled(true);
		_clearMapCacheAction->setEnabled(true);
	} else {
		qWarning("%s: %s", qPrintable(map->path()), qPrintable(map->errorString()));
		action->deleteLater();
	}
}

TreeNode<POIAction *> GUI::createPOIActions()
{
	_poisActionGroup = new QActionGroup(this);
	_poisActionGroup->setExclusive(false);
	connect(_poisActionGroup, &QActionGroup::triggered, this,
	  &GUI::poiFileChecked);

	TreeNode<QString> poiFiles;
	QString poiDir(ProgramPaths::poiDir());
	if (!poiDir.isNull())
		poiFiles = _poi->loadDir(poiDir);

	return createPOIActionsNode(poiFiles);
}

TreeNode<POIAction *> GUI::createPOIActionsNode(const TreeNode<QString> &node)
{
	TreeNode<POIAction*> tree(node.name());

	for (int i = 0; i < node.childs().size(); i++)
		tree.addChild(createPOIActionsNode(node.childs().at(i)));
	for (int i = 0; i < node.items().size(); i++)
		tree.addItem(new POIAction(node.items().at(i), _poisActionGroup));

	return tree;
}

void GUI::createActions(TreeNode<MapAction*> &mapActions,
  TreeNode<POIAction*> &poiActions)
{
	QActionGroup *ag;

	// Action Groups
	_fileActionGroup = new QActionGroup(this);
	_fileActionGroup->setExclusive(false);
	_fileActionGroup->setEnabled(false);

	_navigationActionGroup = new QActionGroup(this);
	_navigationActionGroup->setEnabled(false);

	// General actions
	_exitAction = new QAction(QIcon(QUIT_ICON), tr("Quit"), this);
	_exitAction->setShortcut(QUIT_SHORTCUT);
	_exitAction->setMenuRole(QAction::QuitRole);
	connect(_exitAction, &QAction::triggered, this, &GUI::close);
	addAction(_exitAction);

	// Help & About
	_pathsAction = new QAction(tr("Paths"), this);
	_pathsAction->setMenuRole(QAction::NoRole);
	connect(_pathsAction, &QAction::triggered, this, &GUI::paths);
	_keysAction = new QAction(tr("Keyboard controls"), this);
	_keysAction->setMenuRole(QAction::NoRole);
	connect(_keysAction, &QAction::triggered, this, &GUI::keys);
	_aboutAction = new QAction(QIcon(APP_ICON), tr("About GPXSee"), this);
	_aboutAction->setMenuRole(QAction::AboutRole);
	connect(_aboutAction, &QAction::triggered, this, &GUI::about);

	// File actions
	_openFileAction = new QAction(QIcon(OPEN_FILE_ICON), tr("Open..."), this);
	_openFileAction->setMenuRole(QAction::NoRole);
	_openFileAction->setShortcut(OPEN_SHORTCUT);
	connect(_openFileAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::openFile));
	addAction(_openFileAction);
	_printFileAction = new QAction(QIcon(PRINT_FILE_ICON), tr("Print..."),
	  this);
	_printFileAction->setMenuRole(QAction::NoRole);
	_printFileAction->setActionGroup(_fileActionGroup);
	connect(_printFileAction, &QAction::triggered, this, &GUI::printFile);
	addAction(_printFileAction);
	_exportPDFFileAction = new QAction(QIcon(EXPORT_FILE_ICON),
	  tr("Export to PDF..."), this);
	_exportPDFFileAction->setMenuRole(QAction::NoRole);
	_exportPDFFileAction->setShortcut(PDF_EXPORT_SHORTCUT);
	_exportPDFFileAction->setActionGroup(_fileActionGroup);
	connect(_exportPDFFileAction, &QAction::triggered, this, &GUI::exportPDFFile);
	addAction(_exportPDFFileAction);
	_exportPNGFileAction = new QAction(QIcon(EXPORT_FILE_ICON),
	  tr("Export to PNG..."), this);
	_exportPNGFileAction->setMenuRole(QAction::NoRole);
	_exportPNGFileAction->setShortcut(PNG_EXPORT_SHORTCUT);
	_exportPNGFileAction->setActionGroup(_fileActionGroup);
	connect(_exportPNGFileAction, &QAction::triggered, this, &GUI::exportPNGFile);
	addAction(_exportPNGFileAction);
	_closeFileAction = new QAction(QIcon(CLOSE_FILE_ICON), tr("Close"), this);
	_closeFileAction->setMenuRole(QAction::NoRole);
	_closeFileAction->setShortcut(CLOSE_SHORTCUT);
	_closeFileAction->setActionGroup(_fileActionGroup);
	connect(_closeFileAction, &QAction::triggered, this, &GUI::closeAll);
	addAction(_closeFileAction);
	_reloadFileAction = new QAction(QIcon(RELOAD_FILE_ICON), tr("Reload"),
	  this);
	_reloadFileAction->setMenuRole(QAction::NoRole);
	_reloadFileAction->setShortcut(RELOAD_SHORTCUT);
	_reloadFileAction->setActionGroup(_fileActionGroup);
	connect(_reloadFileAction, &QAction::triggered, this, &GUI::reloadFiles);
	addAction(_reloadFileAction);
	_statisticsAction = new QAction(tr("Statistics..."), this);
	_statisticsAction->setMenuRole(QAction::NoRole);
	_statisticsAction->setShortcut(STATISTICS_SHORTCUT);
	_statisticsAction->setActionGroup(_fileActionGroup);
	connect(_statisticsAction, &QAction::triggered, this, &GUI::statistics);
	addAction(_statisticsAction);

	// POI actions
	poiActions = createPOIActions();
	_openPOIAction = new QAction(QIcon(OPEN_FILE_ICON), tr("Load POI file..."),
	  this);
	_openPOIAction->setMenuRole(QAction::NoRole);
	connect(_openPOIAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::openPOIFile));
	_selectAllPOIAction = new QAction(tr("Select all files"), this);
	_selectAllPOIAction->setMenuRole(QAction::NoRole);
	_selectAllPOIAction->setEnabled(!_poisActionGroup->actions().isEmpty());
	connect(_selectAllPOIAction, &QAction::triggered, this,
	  &GUI::selectAllPOIs);
	_unselectAllPOIAction = new QAction(tr("Unselect all files"), this);
	_unselectAllPOIAction->setMenuRole(QAction::NoRole);
	_unselectAllPOIAction->setEnabled(_selectAllPOIAction->isEnabled());
	connect(_unselectAllPOIAction, &QAction::triggered, this,
	  &GUI::unselectAllPOIs);
	_overlapPOIAction = new QAction(tr("Overlap POIs"), this);
	_overlapPOIAction->setMenuRole(QAction::NoRole);
	_overlapPOIAction->setCheckable(true);
	connect(_overlapPOIAction, &QAction::triggered, _mapView,
	  &MapView::showOverlappedPOIs);
	_showPOILabelsAction = new QAction(tr("Show POI labels"), this);
	_showPOILabelsAction->setMenuRole(QAction::NoRole);
	_showPOILabelsAction->setCheckable(true);
	connect(_showPOILabelsAction, &QAction::triggered, _mapView,
	  &MapView::showPOILabels);
	_showPOIAction = new QAction(QIcon(SHOW_POI_ICON), tr("Show POIs"), this);
	_showPOIAction->setMenuRole(QAction::NoRole);
	_showPOIAction->setCheckable(true);
	_showPOIAction->setShortcut(SHOW_POI_SHORTCUT);
	connect(_showPOIAction, &QAction::triggered, _mapView, &MapView::showPOI);
	addAction(_showPOIAction);

	// Map actions
	mapActions = createMapActions();
	_showMapAction = new QAction(QIcon(SHOW_MAP_ICON), tr("Show map"),
	  this);
	_showMapAction->setEnabled(false);
	_showMapAction->setMenuRole(QAction::NoRole);
	_showMapAction->setCheckable(true);
	_showMapAction->setShortcut(SHOW_MAP_SHORTCUT);
	connect(_showMapAction, &QAction::triggered, _mapView,
	  &MapView::showMap);
	addAction(_showMapAction);
	_loadMapAction = new QAction(QIcon(OPEN_FILE_ICON), tr("Load map..."),
	  this);
	_loadMapAction->setMenuRole(QAction::NoRole);
	connect(_loadMapAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::loadMap));
	_loadMapDirAction = new QAction(QIcon(OPEN_FILE_ICON),
	  tr("Load map directory..."), this);
	_loadMapDirAction->setMenuRole(QAction::NoRole);
	connect(_loadMapDirAction, &QAction::triggered, this, &GUI::loadMapDir);
	_clearMapCacheAction = new QAction(tr("Clear tile cache"), this);
	_clearMapCacheAction->setEnabled(false);
	_clearMapCacheAction->setMenuRole(QAction::NoRole);
	connect(_clearMapCacheAction, &QAction::triggered, this,
	  &GUI::clearMapCache);
	_nextMapAction = new QAction(tr("Next map"), this);
	_nextMapAction->setMenuRole(QAction::NoRole);
	_nextMapAction->setShortcut(NEXT_MAP_SHORTCUT);
	connect(_nextMapAction, &QAction::triggered, this, &GUI::nextMap);
	addAction(_nextMapAction);
	_prevMapAction = new QAction(tr("Next map"), this);
	_prevMapAction->setMenuRole(QAction::NoRole);
	_prevMapAction->setShortcut(PREV_MAP_SHORTCUT);
	connect(_prevMapAction, &QAction::triggered, this, &GUI::prevMap);
	addAction(_prevMapAction);
	_showCoordinatesAction = new QAction(tr("Show cursor coordinates"), this);
	_showCoordinatesAction->setMenuRole(QAction::NoRole);
	_showCoordinatesAction->setCheckable(true);
	connect(_showCoordinatesAction, &QAction::triggered, _mapView,
	  &MapView::showCoordinates);

	// Data actions
	_showTracksAction = new QAction(tr("Show tracks"), this);
	_showTracksAction->setMenuRole(QAction::NoRole);
	_showTracksAction->setCheckable(true);
	connect(_showTracksAction, &QAction::triggered, this, &GUI::showTracks);
	_showRoutesAction = new QAction(tr("Show routes"), this);
	_showRoutesAction->setMenuRole(QAction::NoRole);
	_showRoutesAction->setCheckable(true);
	connect(_showRoutesAction, &QAction::triggered, this, &GUI::showRoutes);
	_showWaypointsAction = new QAction(tr("Show waypoints"), this);
	_showWaypointsAction->setMenuRole(QAction::NoRole);
	_showWaypointsAction->setCheckable(true);
	connect(_showWaypointsAction, &QAction::triggered, _mapView,
	  &MapView::showWaypoints);
	_showAreasAction = new QAction(tr("Show areas"), this);
	_showAreasAction->setMenuRole(QAction::NoRole);
	_showAreasAction->setCheckable(true);
	connect(_showAreasAction, &QAction::triggered, _mapView,
	  &MapView::showAreas);
	_showWaypointLabelsAction = new QAction(tr("Waypoint labels"), this);
	_showWaypointLabelsAction->setMenuRole(QAction::NoRole);
	_showWaypointLabelsAction->setCheckable(true);
	connect(_showWaypointLabelsAction, &QAction::triggered, _mapView,
	  &MapView::showWaypointLabels);
	_showRouteWaypointsAction = new QAction(tr("Route waypoints"), this);
	_showRouteWaypointsAction->setMenuRole(QAction::NoRole);
	_showRouteWaypointsAction->setCheckable(true);
	connect(_showRouteWaypointsAction, &QAction::triggered, _mapView,
	  &MapView::showRouteWaypoints);
	_showTicksAction = new QAction(tr("km/mi markers"), this);
	_showTicksAction->setMenuRole(QAction::NoRole);
	_showTicksAction->setCheckable(true);
	connect(_showTicksAction, &QAction::triggered, _mapView,
	  &MapView::showTicks);
	QActionGroup *markerInfoGroup = new QActionGroup(this);
	connect(markerInfoGroup, &QActionGroup::triggered, this,
	  &GUI::showPathMarkerInfo);
	_hideMarkersAction = new QAction(tr("Do not show"), this);
	_hideMarkersAction->setMenuRole(QAction::NoRole);
	_hideMarkersAction->setCheckable(true);
	_hideMarkersAction->setActionGroup(markerInfoGroup);
	_showMarkersAction = new QAction(tr("Marker only"), this);
	_showMarkersAction->setMenuRole(QAction::NoRole);
	_showMarkersAction->setCheckable(true);
	_showMarkersAction->setActionGroup(markerInfoGroup);
	_showMarkerDateAction = new QAction(tr("Date/time"), this);
	_showMarkerDateAction->setMenuRole(QAction::NoRole);
	_showMarkerDateAction->setCheckable(true);
	_showMarkerDateAction->setActionGroup(markerInfoGroup);
	_showMarkerCoordinatesAction = new QAction(tr("Coordinates"), this);
	_showMarkerCoordinatesAction->setMenuRole(QAction::NoRole);
	_showMarkerCoordinatesAction->setCheckable(true);
	_showMarkerCoordinatesAction->setActionGroup(markerInfoGroup);

	// Graph actions
	_showGraphsAction = new QAction(QIcon(SHOW_GRAPHS_ICON), tr("Show graphs"),
	  this);
	_showGraphsAction->setMenuRole(QAction::NoRole);
	_showGraphsAction->setCheckable(true);
	_showGraphsAction->setShortcut(SHOW_GRAPHS_SHORTCUT);
	connect(_showGraphsAction, &QAction::triggered, this, &GUI::showGraphs);
	addAction(_showGraphsAction);
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	_distanceGraphAction = new QAction(tr("Distance"), this);
	_distanceGraphAction->setMenuRole(QAction::NoRole);
	_distanceGraphAction->setCheckable(true);
	_distanceGraphAction->setActionGroup(ag);
	connect(_distanceGraphAction, &QAction::triggered, this,
	  &GUI::setDistanceGraph);
	addAction(_distanceGraphAction);
	_timeGraphAction = new QAction(tr("Time"), this);
	_timeGraphAction->setMenuRole(QAction::NoRole);
	_timeGraphAction->setCheckable(true);
	_timeGraphAction->setActionGroup(ag);
	connect(_timeGraphAction, &QAction::triggered, this, &GUI::setTimeGraph);
	addAction(_timeGraphAction);
	_showGraphGridAction = new QAction(tr("Show grid"), this);
	_showGraphGridAction->setMenuRole(QAction::NoRole);
	_showGraphGridAction->setCheckable(true);
	connect(_showGraphGridAction, &QAction::triggered, this,
	  &GUI::showGraphGrids);
	_showGraphSliderInfoAction = new QAction(tr("Show slider info"), this);
	_showGraphSliderInfoAction->setMenuRole(QAction::NoRole);
	_showGraphSliderInfoAction->setCheckable(true);
	connect(_showGraphSliderInfoAction, &QAction::triggered, this,
	  &GUI::showGraphSliderInfo);

	// Settings actions
	_showToolbarsAction = new QAction(tr("Show toolbars"), this);
	_showToolbarsAction->setMenuRole(QAction::NoRole);
	_showToolbarsAction->setCheckable(true);
	connect(_showToolbarsAction, &QAction::triggered, this, &GUI::showToolbars);
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	_totalTimeAction = new QAction(tr("Total time"), this);
	_totalTimeAction->setMenuRole(QAction::NoRole);
	_totalTimeAction->setCheckable(true);
	_totalTimeAction->setActionGroup(ag);
	connect(_totalTimeAction, &QAction::triggered, this, &GUI::setTotalTime);
	_movingTimeAction = new QAction(tr("Moving time"), this);
	_movingTimeAction->setMenuRole(QAction::NoRole);
	_movingTimeAction->setCheckable(true);
	_movingTimeAction->setActionGroup(ag);
	connect(_movingTimeAction, &QAction::triggered, this, &GUI::setMovingTime);
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	_metricUnitsAction = new QAction(tr("Metric"), this);
	_metricUnitsAction->setMenuRole(QAction::NoRole);
	_metricUnitsAction->setCheckable(true);
	_metricUnitsAction->setActionGroup(ag);
	connect(_metricUnitsAction, &QAction::triggered, this, &GUI::setMetricUnits);
	_imperialUnitsAction = new QAction(tr("Imperial"), this);
	_imperialUnitsAction->setMenuRole(QAction::NoRole);
	_imperialUnitsAction->setCheckable(true);
	_imperialUnitsAction->setActionGroup(ag);
	connect(_imperialUnitsAction, &QAction::triggered, this,
	  &GUI::setImperialUnits);
	_nauticalUnitsAction = new QAction(tr("Nautical"), this);
	_nauticalUnitsAction->setMenuRole(QAction::NoRole);
	_nauticalUnitsAction->setCheckable(true);
	_nauticalUnitsAction->setActionGroup(ag);
	connect(_nauticalUnitsAction, &QAction::triggered, this,
	  &GUI::setNauticalUnits);
	ag = new QActionGroup(this);
	ag->setExclusive(true);
	_decimalDegreesAction = new QAction(tr("Decimal degrees (DD)"), this);
	_decimalDegreesAction->setMenuRole(QAction::NoRole);
	_decimalDegreesAction->setCheckable(true);
	_decimalDegreesAction->setActionGroup(ag);
	connect(_decimalDegreesAction, &QAction::triggered, this,
	  &GUI::setDecimalDegrees);
	_degreesMinutesAction = new QAction(tr("Degrees and decimal minutes (DMM)"),
	  this);
	_degreesMinutesAction->setMenuRole(QAction::NoRole);
	_degreesMinutesAction->setCheckable(true);
	_degreesMinutesAction->setActionGroup(ag);
	connect(_degreesMinutesAction, &QAction::triggered, this,
	  &GUI::setDegreesMinutes);
	_dmsAction = new QAction(tr("Degrees, minutes, seconds (DMS)"), this);
	_dmsAction->setMenuRole(QAction::NoRole);
	_dmsAction->setCheckable(true);
	_dmsAction->setActionGroup(ag);
	connect(_dmsAction, &QAction::triggered, this, &GUI::setDMS);
	_fullscreenAction = new QAction(QIcon(FULLSCREEN_ICON),
	  tr("Fullscreen mode"), this);
	_fullscreenAction->setMenuRole(QAction::NoRole);
	_fullscreenAction->setCheckable(true);
	_fullscreenAction->setShortcut(FULLSCREEN_SHORTCUT);
	connect(_fullscreenAction, &QAction::triggered, this, &GUI::showFullscreen);
	addAction(_fullscreenAction);
	_openOptionsAction = new QAction(tr("Options..."), this);
	_openOptionsAction->setMenuRole(QAction::PreferencesRole);
	connect(_openOptionsAction, &QAction::triggered, this, &GUI::openOptions);

	// Navigation actions
	_nextAction = new QAction(QIcon(NEXT_FILE_ICON), tr("Next"), this);
	_nextAction->setActionGroup(_navigationActionGroup);
	_nextAction->setMenuRole(QAction::NoRole);
	connect(_nextAction, &QAction::triggered, this, &GUI::next);
	_prevAction = new QAction(QIcon(PREV_FILE_ICON), tr("Previous"), this);
	_prevAction->setMenuRole(QAction::NoRole);
	_prevAction->setActionGroup(_navigationActionGroup);
	connect(_prevAction, &QAction::triggered, this, &GUI::prev);
	_lastAction = new QAction(QIcon(LAST_FILE_ICON), tr("Last"), this);
	_lastAction->setMenuRole(QAction::NoRole);
	_lastAction->setActionGroup(_navigationActionGroup);
	connect(_lastAction, &QAction::triggered, this, &GUI::last);
	_firstAction = new QAction(QIcon(FIRST_FILE_ICON), tr("First"), this);
	_firstAction->setMenuRole(QAction::NoRole);
	_firstAction->setActionGroup(_navigationActionGroup);
	connect(_firstAction, &QAction::triggered, this, &GUI::first);
}

void GUI::createMapNodeMenu(const TreeNode<MapAction*> &node, QMenu *menu)
{
	for (int i = 0; i < node.childs().size(); i++) {
		QMenu *cm = new QMenu(node.childs().at(i).name(), menu);
		menu->addMenu(cm);
		createMapNodeMenu(node.childs().at(i), cm);
	}

	for (int i = 0; i < node.items().size(); i++)
		menu->addAction(node.items().at(i));
}

void GUI::createPOINodeMenu(const TreeNode<POIAction*> &node, QMenu *menu)
{
	for (int i = 0; i < node.childs().size(); i++) {
		QMenu *cm = new QMenu(node.childs().at(i).name(), menu);
		menu->addMenu(cm);
		createPOINodeMenu(node.childs().at(i), cm);
	}

	for (int i = 0; i < node.items().size(); i++)
		menu->addAction(node.items().at(i));
}

void GUI::createMenus(const TreeNode<MapAction*> &mapActions,
  const TreeNode<POIAction*> &poiActions)
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(_openFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_printFileAction);
	fileMenu->addAction(_exportPDFFileAction);
	fileMenu->addAction(_exportPNGFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_statisticsAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_reloadFileAction);
	fileMenu->addAction(_closeFileAction);
#ifndef Q_OS_MAC
	fileMenu->addSeparator();
	fileMenu->addAction(_exitAction);
#endif // Q_OS_MAC

	_mapMenu = menuBar()->addMenu(tr("&Map"));
	createMapNodeMenu(mapActions, _mapMenu);
	_mapsEnd = _mapMenu->addSeparator();
	_mapMenu->addAction(_loadMapAction);
	_mapMenu->addAction(_loadMapDirAction);
	_mapMenu->addAction(_clearMapCacheAction);
	_mapMenu->addSeparator();
	_mapMenu->addAction(_showCoordinatesAction);
	_mapMenu->addSeparator();
	_mapMenu->addAction(_showMapAction);

	QMenu *graphMenu = menuBar()->addMenu(tr("&Graph"));
	graphMenu->addAction(_distanceGraphAction);
	graphMenu->addAction(_timeGraphAction);
	graphMenu->addSeparator();
	graphMenu->addAction(_showGraphGridAction);
	graphMenu->addAction(_showGraphSliderInfoAction);
	graphMenu->addSeparator();
	graphMenu->addAction(_showGraphsAction);

	_poiMenu = menuBar()->addMenu(tr("&POI"));
	createPOINodeMenu(poiActions, _poiMenu);
	_poisEnd = _poiMenu->addSeparator();
	_poiMenu->addAction(_openPOIAction);
	_poiMenu->addAction(_selectAllPOIAction);
	_poiMenu->addAction(_unselectAllPOIAction);
	_poiMenu->addSeparator();
	_poiMenu->addAction(_showPOILabelsAction);
	_poiMenu->addAction(_overlapPOIAction);
	_poiMenu->addSeparator();
	_poiMenu->addAction(_showPOIAction);

	QMenu *dataMenu = menuBar()->addMenu(tr("&Data"));
	dataMenu->addAction(_showWaypointLabelsAction);
	dataMenu->addAction(_showRouteWaypointsAction);
	dataMenu->addAction(_showTicksAction);
	QMenu *markerMenu = dataMenu->addMenu(tr("Position info"));
	markerMenu->addAction(_hideMarkersAction);
	markerMenu->addAction(_showMarkersAction);
	markerMenu->addAction(_showMarkerDateAction);
	markerMenu->addAction(_showMarkerCoordinatesAction);
	dataMenu->addSeparator();
	dataMenu->addAction(_showTracksAction);
	dataMenu->addAction(_showRoutesAction);
	dataMenu->addAction(_showAreasAction);
	dataMenu->addAction(_showWaypointsAction);

	QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
	QMenu *timeMenu = settingsMenu->addMenu(tr("Time"));
	timeMenu->addAction(_totalTimeAction);
	timeMenu->addAction(_movingTimeAction);
	QMenu *unitsMenu = settingsMenu->addMenu(tr("Units"));
	unitsMenu->addAction(_metricUnitsAction);
	unitsMenu->addAction(_imperialUnitsAction);
	unitsMenu->addAction(_nauticalUnitsAction);
	QMenu *coordinatesMenu = settingsMenu->addMenu(tr("Coordinates format"));
	coordinatesMenu->addAction(_decimalDegreesAction);
	coordinatesMenu->addAction(_degreesMinutesAction);
	coordinatesMenu->addAction(_dmsAction);
	settingsMenu->addSeparator();
	settingsMenu->addAction(_showToolbarsAction);
	settingsMenu->addAction(_fullscreenAction);
	settingsMenu->addSeparator();
	settingsMenu->addAction(_openOptionsAction);

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(_pathsAction);
	helpMenu->addAction(_keysAction);
	helpMenu->addSeparator();
	helpMenu->addAction(_aboutAction);
}

void GUI::createToolBars()
{
	int is = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
	QSize iconSize(qMin(is, TOOLBAR_ICON_SIZE), qMin(is, TOOLBAR_ICON_SIZE));

#ifdef Q_OS_MAC
	setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // Q_OS_MAC

	_fileToolBar = addToolBar(tr("File"));
	_fileToolBar->setObjectName("File");
	_fileToolBar->setIconSize(iconSize);
	_fileToolBar->addAction(_openFileAction);
	_fileToolBar->addAction(_reloadFileAction);
	_fileToolBar->addAction(_closeFileAction);
	_fileToolBar->addAction(_printFileAction);

	_showToolBar = addToolBar(tr("Show"));
	_showToolBar->setObjectName("Show");
	_showToolBar->setIconSize(iconSize);
	_showToolBar->addAction(_showPOIAction);
	_showToolBar->addAction(_showMapAction);
	_showToolBar->addAction(_showGraphsAction);

	_navigationToolBar = addToolBar(tr("Navigation"));
	_navigationToolBar->setObjectName("Navigation");
	_navigationToolBar->setIconSize(iconSize);
	_navigationToolBar->addAction(_firstAction);
	_navigationToolBar->addAction(_prevAction);
	_navigationToolBar->addAction(_nextAction);
	_navigationToolBar->addAction(_lastAction);
}

void GUI::createMapView()
{
	_map = new EmptyMap(this);
	_mapView = new MapView(_map, _poi, this);
	_mapView->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,
	  QSizePolicy::Expanding));
	_mapView->setMinimumHeight(200);
#ifdef Q_OS_WIN32
	_mapView->setFrameShape(QFrame::NoFrame);
#endif // Q_OS_WIN32
}

void GUI::createGraphTabs()
{
	_graphTabWidget = new QTabWidget();
	_graphTabWidget->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,
	  QSizePolicy::Preferred));
	_graphTabWidget->setMinimumHeight(200);
#ifndef Q_OS_MAC
	_graphTabWidget->setDocumentMode(true);
#endif // Q_OS_MAC

	connect(_graphTabWidget, &QTabWidget::currentChanged, this,
	  &GUI::graphChanged);

	_tabs.append(new ElevationGraph(_graphTabWidget));
	_tabs.append(new SpeedGraph(_graphTabWidget));
	_tabs.append(new HeartRateGraph(_graphTabWidget));
	_tabs.append(new CadenceGraph(_graphTabWidget));
	_tabs.append(new PowerGraph(_graphTabWidget));
	_tabs.append(new TemperatureGraph(_graphTabWidget));
	_tabs.append(new GearRatioGraph(_graphTabWidget));

	for (int i = 0; i < _tabs.size(); i++)
		connect(_tabs.at(i), &GraphTab::sliderPositionChanged, _mapView,
		  &MapView::setMarkerPosition);
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
	QUrl homepage(APP_HOMEPAGE);

	msgBox.setWindowTitle(tr("About GPXSee"));
	msgBox.setText("<h2>" + QString(APP_NAME) + "</h2><p><p>" + tr("Version %1")
	  .arg(QString(APP_VERSION) + " (" + QSysInfo::buildCpuArchitecture()
	  + ", Qt " + QT_VERSION_STR + ")") + "</p>");
	msgBox.setInformativeText("<table width=\"300\"><tr><td>"
	  + tr("GPXSee is distributed under the terms of the GNU General Public "
	  "License version 3. For more info about GPXSee visit the project "
	  "homepage at %1.").arg("<a href=\"" + homepage.toString() + "\">"
	  + homepage.toString(QUrl::RemoveScheme).mid(2) + "</a>")
	  + "</td></tr></table>");

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
	  + "</i></td></tr><tr><td></td><td></td></tr><tr><td>"
	  + tr("Toggle graph type") + "</td><td><i>"
	  + QKeySequence(TOGGLE_GRAPH_TYPE_KEY).toString() + "</i></td></tr><tr><td>"
	  + tr("Toggle time type") + "</td><td><i>"
	  + QKeySequence(TOGGLE_TIME_TYPE_KEY).toString() + "</i></td></tr><tr><td>"
	  + tr("Toggle position info") + "</td><td><i>"
	  + QKeySequence(TOGGLE_MARKER_INFO_KEY).toString() + "</i></td></tr>"
	  + "<tr><td></td><td></td></tr><tr><td>" + tr("Next map")
	  + "</td><td><i>" + NEXT_MAP_SHORTCUT.toString() + "</i></td></tr><tr><td>"
	  + tr("Previous map") + "</td><td><i>" + PREV_MAP_SHORTCUT.toString()
	  + "</i></td></tr><tr><td></td><td></td></tr><tr><td>" + tr("Zoom in")
	  + "</td><td><i>" + QKeySequence(ZOOM_IN).toString()
	  + "</i></td></tr><tr><td>" + tr("Zoom out") + "</td><td><i>"
	  + QKeySequence(ZOOM_OUT).toString() + "</i></td></tr><tr><td>"
	  + tr("Digital zoom") + "</td><td><i>" + QKeySequence(MODIFIER).toString()
	  + tr("Zoom") + "</i></td></tr><tr><td></td><td></td></tr><tr><td>"
	  + tr("Copy coordinates") + "</td><td><i>"
	  + QKeySequence(MODIFIER).toString() + tr("Left Click")
	  + "</i></td></tr></table></div>");

	msgBox.exec();
}

void GUI::paths()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("Paths"));
	msgBox.setText("<h3>" + tr("Paths") + "</h3>");
	msgBox.setInformativeText(
	  "<style>td {white-space: pre; padding-right: 1em;}</style><table><tr><td>"
	  + tr("Map directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::mapDir(true)) + "</code></td></tr><tr><td>"
	  + tr("POI directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::poiDir(true)) + "</code></td></tr><tr><td>"
	  + tr("GCS/PCS directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::csvDir(true)) + "</code></td></tr><tr><td>"
	  + tr("DEM directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::demDir(true)) + "</code></td></tr><tr><td>"
	  + tr("Styles directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::styleDir(true)) + "</code></td></tr><tr><td>"
	  + tr("Tile cache directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::tilesDir()) + "</code></td></tr></table>"
	);

	msgBox.exec();
}

void GUI::openFile()
{
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open file"),
	  _dataDir, Data::formats()));

	for (int i = 0; i < files.size(); i++)
		openFile(files.at(i));
	if (!files.isEmpty())
		_dataDir = QFileInfo(files.last()).path();
}

bool GUI::openFile(const QString &fileName, bool silent)
{
	if (_files.contains(fileName))
		return true;

	if (!loadFile(fileName, silent))
		return false;

	_files.append(fileName);
	_browser->setCurrent(fileName);
	_fileActionGroup->setEnabled(true);
	// Explicitly enable the reload action as it may be disabled by loadMapDir()
	_reloadFileAction->setEnabled(true);
	_navigationActionGroup->setEnabled(true);

	updateNavigationActions();
	updateStatusBarInfo();
	updateWindowTitle();

	return true;
}

bool GUI::loadFile(const QString &fileName, bool silent)
{
	Data data(fileName, !silent);

	if (data.isValid()) {
		loadData(data);
		return true;
	} else if (!silent) {
		updateNavigationActions();
		updateStatusBarInfo();
		updateWindowTitle();
		updateGraphTabs();

		QString error = tr("Error loading data file:") + "\n\n"
		  + fileName + "\n\n" + data.errorString();
		if (data.errorLine())
			error.append("\n" + tr("Line: %1").arg(data.errorLine()));
		QMessageBox::critical(this, APP_NAME, error);
		return false;
	} else
		return false;
}

void GUI::loadData(const Data &data)
{
	QList<QList<GraphItem*> > graphs;
	QList<PathItem*> paths;

	for (int i = 0; i < data.tracks().count(); i++) {
		const Track &track = data.tracks().at(i);
		_trackDistance += track.distance();
		_time += track.time();
		_movingTime += track.movingTime();
		const QDateTime date = track.date().toTimeZone(_options.timeZone.zone());
		if (_dateRange.first.isNull() || _dateRange.first > date)
			_dateRange.first = date;
		if (_dateRange.second.isNull() || _dateRange.second < date)
			_dateRange.second = date;
	}
	_trackCount += data.tracks().count();

	for (int i = 0; i < data.routes().count(); i++)
		_routeDistance += data.routes().at(i).distance();
	_routeCount += data.routes().count();

	_waypointCount += data.waypoints().count();
	_areaCount += data.areas().count();

	if (_pathName.isNull()) {
		if (data.tracks().count() == 1 && !data.routes().count())
			_pathName = data.tracks().first().name();
		else if (data.routes().count() == 1 && !data.tracks().count())
			_pathName = data.routes().first().name();
	} else
		_pathName = QString();

	for (int i = 0; i < _tabs.count(); i++)
		graphs.append(_tabs.at(i)->loadData(data));
	if (updateGraphTabs())
		_splitter->refresh();
	paths = _mapView->loadData(data);

	GraphTab *gt = static_cast<GraphTab*>(_graphTabWidget->currentWidget());

	for (int i = 0; i < paths.count(); i++) {
		PathItem *pi = paths.at(i);
		if (!pi)
			continue;

		for (int j = 0; j < graphs.count(); j++)
			pi->addGraph(graphs.at(j).at(i));

		if (gt) {
			pi->setGraph(_tabs.indexOf(gt));
			pi->setMarkerPosition(gt->sliderPosition());
		}
	}
}

void GUI::openPOIFile()
{
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open POI file"),
	  _poiDir, Data::formats()));

	for (int i = 0; i < files.size(); i++)
		openPOIFile(files.at(i));
	if (!files.isEmpty())
		_poiDir = QFileInfo(files.last()).path();
}

bool GUI::openPOIFile(const QString &fileName)
{
	if (_poi->isLoaded(fileName))
		return true;

	if (_poi->loadFile(fileName)) {
		_mapView->showPOI(true);
		_showPOIAction->setChecked(true);
		QAction *action = new POIAction(fileName, _poisActionGroup);
		action->setChecked(true);
		_poiMenu->insertAction(_poisEnd, action);

		_selectAllPOIAction->setEnabled(true);
		_unselectAllPOIAction->setEnabled(true);

		return true;
	} else {
		QString error = tr("Error loading POI file:") + "\n\n"
		  + fileName + "\n\n" + _poi->errorString();
		if (_poi->errorLine())
			error.append("\n" + tr("Line: %1").arg(_poi->errorLine()));
		QMessageBox::critical(this, APP_NAME, error);

		return false;
	}
}

void GUI::openOptions()
{
#define SET_VIEW_OPTION(option, action) \
	if (options.option != _options.option) \
		_mapView->action(options.option)
#define SET_TAB_OPTION(option, action) \
	if (options.option != _options.option) \
		for (int i = 0; i < _tabs.count(); i++) \
			_tabs.at(i)->action(options.option)
#define SET_TRACK_OPTION(option, action) \
	if (options.option != _options.option) { \
		Track::action(options.option); \
		reload = true; \
	}
#define SET_ROUTE_OPTION(option, action) \
	if (options.option != _options.option) { \
		Route::action(options.option); \
		reload = true; \
	}
#define SET_WAYPOINT_OPTION(option, action) \
	if (options.option != _options.option) { \
		Waypoint::action(options.option); \
		reload = true; \
	}

	Options options(_options);
	bool reload = false;

	OptionsDialog dialog(options, _units, this);
	if (dialog.exec() != QDialog::Accepted)
		return;

	SET_VIEW_OPTION(palette, setPalette);
	SET_VIEW_OPTION(mapOpacity, setMapOpacity);
	SET_VIEW_OPTION(backgroundColor, setBackgroundColor);
	SET_VIEW_OPTION(trackWidth, setTrackWidth);
	SET_VIEW_OPTION(routeWidth, setRouteWidth);
	SET_VIEW_OPTION(areaWidth, setAreaWidth);
	SET_VIEW_OPTION(trackStyle, setTrackStyle);
	SET_VIEW_OPTION(routeStyle, setRouteStyle);
	SET_VIEW_OPTION(areaStyle, setAreaStyle);
	SET_VIEW_OPTION(areaOpacity, setAreaOpacity);
	SET_VIEW_OPTION(waypointSize, setWaypointSize);
	SET_VIEW_OPTION(waypointColor, setWaypointColor);
	SET_VIEW_OPTION(poiSize, setPOISize);
	SET_VIEW_OPTION(poiColor, setPOIColor);
	SET_VIEW_OPTION(pathAntiAliasing, useAntiAliasing);
	SET_VIEW_OPTION(useOpenGL, useOpenGL);
	SET_VIEW_OPTION(sliderColor, setMarkerColor);

	if (options.hidpiMap != _options.hidpiMap)
		_mapView->setDevicePixelRatio(devicePixelRatioF(),
		  options.hidpiMap ? devicePixelRatioF() : 1.0);
	if (options.outputProjection != _options.outputProjection)
		_mapView->setOutputProjection(CRS::projection(options.outputProjection));
	if (options.inputProjection != _options.inputProjection)
		_mapView->setInputProjection(CRS::projection(options.inputProjection));
	if (options.timeZone != _options.timeZone) {
		_mapView->setTimeZone(options.timeZone.zone());
		_dateRange.first = _dateRange.first.toTimeZone(options.timeZone.zone());
		_dateRange.second = _dateRange.second.toTimeZone(options.timeZone.zone());
	}

	SET_TAB_OPTION(palette, setPalette);
	SET_TAB_OPTION(graphWidth, setGraphWidth);
	SET_TAB_OPTION(graphAntiAliasing, useAntiAliasing);
	SET_TAB_OPTION(useOpenGL, useOpenGL);
	SET_TAB_OPTION(sliderColor, setSliderColor);

	SET_TRACK_OPTION(elevationFilter, setElevationFilter);
	SET_TRACK_OPTION(speedFilter, setSpeedFilter);
	SET_TRACK_OPTION(heartRateFilter, setHeartRateFilter);
	SET_TRACK_OPTION(cadenceFilter, setCadenceFilter);
	SET_TRACK_OPTION(powerFilter, setPowerFilter);
	SET_TRACK_OPTION(outlierEliminate, setOutlierElimination);
	SET_TRACK_OPTION(automaticPause, setAutomaticPause);
	SET_TRACK_OPTION(pauseSpeed, setPauseSpeed);
	SET_TRACK_OPTION(pauseInterval, setPauseInterval);
	SET_TRACK_OPTION(useReportedSpeed, useReportedSpeed);
	SET_TRACK_OPTION(dataUseDEM, useDEM);
	SET_TRACK_OPTION(showSecondaryElevation, showSecondaryElevation);
	SET_TRACK_OPTION(showSecondarySpeed, showSecondarySpeed);
	SET_TRACK_OPTION(useSegments, useSegments);

	SET_ROUTE_OPTION(dataUseDEM, useDEM);
	SET_ROUTE_OPTION(showSecondaryElevation, showSecondaryElevation);

	SET_WAYPOINT_OPTION(dataUseDEM, useDEM);
	SET_WAYPOINT_OPTION(showSecondaryElevation, showSecondaryElevation);

	if (options.poiRadius != _options.poiRadius)
		_poi->setRadius(options.poiRadius);

	if (options.pixmapCache != _options.pixmapCache)
		QPixmapCache::setCacheLimit(options.pixmapCache * 1024);

	if (options.connectionTimeout != _options.connectionTimeout)
		Downloader::setTimeout(options.connectionTimeout);
	if (options.enableHTTP2 != _options.enableHTTP2)
		Downloader::enableHTTP2(options.enableHTTP2);

	if (options.dataPath != _options.dataPath)
		_dataDir = options.dataPath;
	if (options.mapsPath != _options.mapsPath)
		_mapDir = options.mapsPath;
	if (options.poiPath != _options.poiPath)
		_poiDir = options.poiPath;

	if (reload)
		reloadFiles();

	_options = options;
}

void GUI::printFile()
{
	QPrinter printer(QPrinter::HighResolution);
	QPrintDialog dialog(&printer, this);

	if (dialog.exec() == QDialog::Accepted)
		plot(&printer);
}

void GUI::exportPDFFile()
{
	PDFExportDialog dialog(_pdfExport, _units, this);
	if (dialog.exec() != QDialog::Accepted)
		return;

	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setCreator(QString(APP_NAME) + QString(" ")
	  + QString(APP_VERSION));
	printer.setResolution(_pdfExport.resolution);
	printer.setPageLayout(QPageLayout(QPageSize(_pdfExport.paperSize),
	  _pdfExport.orientation, _pdfExport.margins, QPageLayout::Millimeter));
	printer.setOutputFileName(_pdfExport.fileName);

	plot(&printer);
}

void GUI::exportPNGFile()
{
	PNGExportDialog dialog(_pngExport, this);
	if (dialog.exec() != QDialog::Accepted)
		return;

	QImage img(_pngExport.size, QImage::Format_ARGB32_Premultiplied);
	QPainter p(&img);
	QRectF rect(0, 0, img.width(), img.height());
	QRectF contentRect(rect.adjusted(_pngExport.margins.left(),
	  _pngExport.margins.top(), -_pngExport.margins.right(),
	  -_pngExport.margins.bottom()));

	if (_pngExport.antialiasing)
		p.setRenderHint(QPainter::Antialiasing);
	p.fillRect(rect, Qt::white);
	plotMainPage(&p, contentRect, 1.0, true);
	img.save(_pngExport.fileName, "png");

	if (!_tabs.isEmpty() && _options.separateGraphPage) {
		QImage img2(_pngExport.size.width(), (int)graphPlotHeight(rect, 1)
		  + _pngExport.margins.bottom(), QImage::Format_ARGB32_Premultiplied);
		QPainter p2(&img2);
		QRectF rect2(0, 0, img2.width(), img2.height());

		if (_pngExport.antialiasing)
			p2.setRenderHint(QPainter::Antialiasing);
		p2.fillRect(rect2, Qt::white);
		plotGraphsPage(&p2, contentRect, 1);

		QFileInfo fi(_pngExport.fileName);
		img2.save(fi.absolutePath() + "/" + fi.baseName() + "-graphs."
		  + fi.suffix(), "png");
	}
}

void GUI::statistics()
{
	QLocale l(QLocale::system());

#ifdef Q_OS_WIN32
	QString text = "<style>td {white-space: pre; padding-right: 4em;}"
	  "th {text-align: left; padding-top: 0.5em;}</style><table>";
#else // Q_OS_WIN32
	QString text = "<style>td {white-space: pre; padding-right: 2em;}"
	  "th {text-align: left; padding-top: 0.5em;}</style><table>";
#endif // Q_OS_WIN32

	if (_showTracksAction->isChecked() && _trackCount > 1)
		text.append("<tr><td>" + tr("Tracks") + ":</td><td>"
		  + l.toString(_trackCount) + "</td></tr>");
	if (_showRoutesAction->isChecked() && _routeCount > 1)
		text.append("<tr><td>" + tr("Routes") + ":</td><td>"
		  + l.toString(_routeCount) + "</td></tr>");
	if (_showWaypointsAction->isChecked() && _waypointCount > 1)
		text.append("<tr><td>" + tr("Waypoints") + ":</td><td>"
		  + l.toString(_waypointCount) + "</td></tr>");
	if (_showAreasAction->isChecked() && _areaCount > 1)
		text.append("<tr><td>" + tr("Areas") + ":</td><td>"
		  + l.toString(_areaCount) + "</td></tr>");

	if (_dateRange.first.isValid()) {
		if (_dateRange.first == _dateRange.second) {
			QString format = l.dateFormat(QLocale::LongFormat);
			text.append("<tr><td>" + tr("Date") + ":</td><td>"
			  + _dateRange.first.toString(format) + "</td></tr>");
		} else {
			QString format = l.dateFormat(QLocale::ShortFormat);
			text.append("<tr><td>" + tr("Date") + ":</td><td>"
			  + QString("%1 - %2").arg(_dateRange.first.toString(format),
			  _dateRange.second.toString(format)) + "</td></tr>");
		}
	}

	if (distance() > 0)
		text.append("<tr><td>" + tr("Distance") + ":</td><td>"
		  + Format::distance(distance(), units()) + "</td></tr>");
	if (time() > 0) {
		text.append("<tr><td>" + tr("Time") + ":</td><td>"
		  + Format::timeSpan(time()) + "</td></tr>");
		text.append("<tr><td>" + tr("Moving time") + ":</td><td>"
		  + Format::timeSpan(movingTime()) + "</td></tr>");
	}

	for (int i = 0; i < _tabs.count(); i++) {
		const GraphTab *tab = _tabs.at(i);
		if (tab->isEmpty())
			continue;

		text.append("<tr><th colspan=\"2\">" + tab->label() + "</th></tr>");
		for (int j = 0; j < tab->info().size(); j++) {
			const KV<QString, QString> &kv = tab->info().at(j);
			text.append("<tr><td>" + kv.key() + ":</td><td>" + kv.value()
			  + "</td></tr>");
		}
	}

	text.append("</table>");


	QMessageBox msgBox(this);
	msgBox.setWindowTitle(tr("Statistics"));
	msgBox.setText("<h3>" + tr("Statistics") + "</h3>");
	msgBox.setInformativeText(text);
	msgBox.exec();
}

void GUI::plotMainPage(QPainter *painter, const QRectF &rect, qreal ratio,
  bool expand)
{
	QLocale l(QLocale::system());
	TrackInfo info;
	qreal ih, gh, mh;
	int sc;


	if (!_pathName.isNull() && _options.printName)
		info.insert(tr("Name"), _pathName);

	if (_options.printItemCount) {
		if (_showTracksAction->isChecked() && _trackCount > 1)
			info.insert(tr("Tracks"), l.toString(_trackCount));
		if (_showRoutesAction->isChecked() && _routeCount > 1)
			info.insert(tr("Routes"), l.toString(_routeCount));
		if (_showWaypointsAction->isChecked() && _waypointCount > 1)
			info.insert(tr("Waypoints"), l.toString(_waypointCount));
		if (_showAreasAction->isChecked() && _areaCount > 1)
			info.insert(tr("Areas"), l.toString(_areaCount));
	}

	if (_dateRange.first.isValid() && _options.printDate) {
		if (_dateRange.first == _dateRange.second) {
			QString format = l.dateFormat(QLocale::LongFormat);
			info.insert(tr("Date"), _dateRange.first.toString(format));
		} else {
			QString format = l.dateFormat(QLocale::ShortFormat);
			info.insert(tr("Date"), QString("%1 - %2")
			  .arg(_dateRange.first.toString(format),
			  _dateRange.second.toString(format)));
		}
	}

	if (distance() > 0 && _options.printDistance)
		info.insert(tr("Distance"), Format::distance(distance(), units()));
	if (time() > 0 && _options.printTime)
		info.insert(tr("Time"), Format::timeSpan(time()));
	if (movingTime() > 0 && _options.printMovingTime)
		info.insert(tr("Moving time"), Format::timeSpan(movingTime()));

	if (info.isEmpty()) {
		ih = 0;
		mh = 0;
	} else {
		ih = info.contentSize().height() * ratio;
		mh = ih / 2;
		info.plot(painter, QRectF(rect.x(), rect.y(), rect.width(), ih), ratio);
	}
	if (_graphTabWidget->isVisible() && !_options.separateGraphPage) {
		qreal r = rect.width() / rect.height();
		gh = (rect.width() > rect.height())
		  ? 0.15 * r * (rect.height() - ih - 2*mh)
		  : 0.15 * (rect.height() - ih - 2*mh);
		if (gh < 150)
			gh = 150;
		sc = 2;
		GraphTab *gt = static_cast<GraphTab*>(_graphTabWidget->currentWidget());
		gt->plot(painter,  QRectF(rect.x(), rect.y() + rect.height() - gh,
		  rect.width(), gh), ratio);
	} else {
		gh = 0;
		sc = 1;
	}

	MapView::PlotFlags flags;
	if (_options.hiresPrint)
		flags |= MapView::HiRes;
	if (expand)
		flags |= MapView::Expand;

	_mapView->plot(painter, QRectF(rect.x(), rect.y() + ih + mh, rect.width(),
	  rect.height() - (ih + sc*mh + gh)), ratio, flags);
}

void GUI::plotGraphsPage(QPainter *painter, const QRectF &rect, qreal ratio)
{
	int cnt = 0;
	for (int i = 0; i < _tabs.size(); i++)
		if (!_tabs.at(i)->isEmpty())
			cnt++;

	qreal sp = ratio * 20;
	qreal gh = qMin((rect.height() - ((cnt - 1) * sp))/(qreal)cnt,
	  0.20 * rect.height());

	qreal y = 0;
	for (int i = 0; i < _tabs.size(); i++) {
		if (!_tabs.at(i)->isEmpty()) {
			_tabs.at(i)->plot(painter,  QRectF(rect.x(), rect.y() + y,
			  rect.width(), gh), ratio);
			y += gh + sp;
		}
	}
}

qreal GUI::graphPlotHeight(const QRectF &rect, qreal ratio)
{
	int cnt = 0;
	for (int i = 0; i < _tabs.size(); i++)
		if (!_tabs.at(i)->isEmpty())
			cnt++;

	qreal sp = ratio * 20;
	qreal gh = qMin((rect.height() - ((cnt - 1) * sp))/(qreal)cnt,
	  0.20 * rect.height());

	return cnt * gh + (cnt - 1) * sp;
}

void GUI::plot(QPrinter *printer)
{
	QPainter p(printer);
	qreal fsr = 1085.0 / (qMax(printer->width(), printer->height())
	  / (qreal)printer->resolution());
	qreal ratio = p.paintEngine()->paintDevice()->logicalDpiX() / fsr;
	QRectF rect(0, 0, printer->width(), printer->height());

	plotMainPage(&p, rect, ratio);

	if (!_tabs.isEmpty() && _options.separateGraphPage) {
		printer->newPage();
		plotGraphsPage(&p, rect, ratio);
	}
}

void GUI::reloadFiles()
{
	_trackCount = 0;
	_routeCount = 0;
	_waypointCount = 0;
	_areaCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_movingTime = 0;
	_dateRange = DateTimeRange(QDateTime(), QDateTime());
	_pathName = QString();

	for (int i = 0; i < _tabs.count(); i++)
		_tabs.at(i)->clear();
	_mapView->clear();

	for (int i = 0; i < _files.size(); i++) {
		if (!loadFile(_files.at(i))) {
			_files.removeAt(i);
			i--;
		}
	}

	updateStatusBarInfo();
	updateWindowTitle();
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
	_areaCount = 0;
	_trackDistance = 0;
	_routeDistance = 0;
	_time = 0;
	_movingTime = 0;
	_dateRange = DateTimeRange(QDateTime(), QDateTime());
	_pathName = QString();

	for (int i = 0; i < _tabs.count(); i++)
		_tabs.at(i)->clear();
	_lastTab = 0;

	_mapView->clear();

	_files.clear();
}

void GUI::closeAll()
{
	closeFiles();

	_fileActionGroup->setEnabled(false);
	updateStatusBarInfo();
	updateWindowTitle();
	updateGraphTabs();
}

void GUI::showGraphs(bool show)
{
	_graphTabWidget->setHidden(!show);
}

void GUI::showToolbars(bool show)
{
	if (show) {
		Q_ASSERT(!_windowStates.isEmpty());
		restoreState(_windowStates.last());
		_windowStates.removeLast();
	} else {
		_windowStates.append(saveState());
		removeToolBar(_fileToolBar);
		removeToolBar(_showToolBar);
		removeToolBar(_navigationToolBar);
	}
}

void GUI::showFullscreen(bool show)
{
	if (show) {
		_windowGeometries.append(saveGeometry());
		_frameStyle = _mapView->frameStyle();
		statusBar()->hide();
		menuBar()->hide();
		showToolbars(false);
		_mapView->setFrameStyle(QFrame::NoFrame);
		_graphTabWidget->tabBar()->hide();
#ifdef Q_OS_MAC
		_graphTabWidget->setDocumentMode(true);
#endif // Q_OS_MAC
		showFullScreen();
	} else {
		Q_ASSERT(!_windowGeometries.isEmpty());
		_windowGeometries.removeLast();
		statusBar()->show();
		menuBar()->show();
		showToolbars(true);
		_mapView->setFrameStyle(_frameStyle);
		_graphTabWidget->tabBar()->show();
#ifdef Q_OS_MAC
		_graphTabWidget->setDocumentMode(false);
#endif // Q_OS_MAC
		showNormal();
	}
}

void GUI::showTracks(bool show)
{
	_mapView->showTracks(show);

	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showTracks(show);

	updateStatusBarInfo();
	updateGraphTabs();
}

void GUI::showRoutes(bool show)
{
	_mapView->showRoutes(show);

	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showRoutes(show);

	updateStatusBarInfo();
	updateGraphTabs();
}

void GUI::showGraphGrids(bool show)
{
	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showGrid(show);
}

void GUI::showGraphSliderInfo(bool show)
{
	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showSliderInfo(show);
}

void GUI::showPathMarkerInfo(QAction *action)
{
	if (action == _showMarkersAction) {
		_mapView->showMarkers(true);
		_mapView->showMarkerInfo(MarkerInfoItem::None);
	} else if (action == _showMarkerDateAction) {
		_mapView->showMarkers(true);
		_mapView->showMarkerInfo(MarkerInfoItem::Date);
	} else if (action == _showMarkerCoordinatesAction) {
		_mapView->showMarkers(true);
		_mapView->showMarkerInfo(MarkerInfoItem::Position);
	} else {
		_mapView->showMarkers(false);
		_mapView->showMarkerInfo(MarkerInfoItem::None);
	}
}

void GUI::loadMap()
{
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open map file"),
	  _mapDir, MapList::formats()));
	MapAction *a, *lastReady = 0;

	for (int i = 0; i < files.size(); i++) {
		if (loadMap(files.at(i), a) && a)
			lastReady = a;
	}
	if (!files.isEmpty())
		_mapDir = QFileInfo(files.last()).path();
	if (lastReady)
		lastReady->trigger();
}

static MapAction *findMapAction(const QList<QAction*> &mapActions,
  const Map *map)
{
	for (int i = 0; i < mapActions.count(); i++) {
		const Map *m = mapActions.at(i)->data().value<Map*>();
		if (map->path() == m->path())
			return static_cast<MapAction*>(mapActions.at(i));
	}

	return 0;
}

bool GUI::loadMapNode(const TreeNode<Map*> &node, MapAction *&action,
  bool silent, const QList<QAction*> &existingActions)
{
	bool valid = false;

	action = 0;

	for (int i = 0; i < node.childs().size(); i++)
		valid = loadMapNode(node.childs().at(i), action, silent, existingActions);

	for (int i = 0; i < node.items().size(); i++) {
		Map *map = node.items().at(i);
		MapAction *a;

		if (!(a = findMapAction(existingActions, map))) {
			if (!map->isValid()) {
				if (!silent)
					QMessageBox::critical(this, APP_NAME,
					  tr("Error loading map:") + "\n\n" + map->path() + "\n\n"
					  + map->errorString());
				delete map;
			} else {
				valid = true;
				a = new MapAction(map, _mapsActionGroup);
				_mapMenu->insertAction(_mapsEnd, a);

				if (map->isReady()) {
					action = a;
					_showMapAction->setEnabled(true);
					_clearMapCacheAction->setEnabled(true);
				} else
					connect(a, &MapAction::loaded, this, &GUI::mapLoaded);
			}
		} else {
			valid = true;
			map = a->data().value<Map*>();
			if (map->isReady())
				action = a;
		}
	}

	return valid;
}

bool GUI::loadMap(const QString &fileName, MapAction *&action, bool silent)
{
	TreeNode<Map*> maps(MapList::loadMaps(fileName,
	  CRS::projection(_options.inputProjection)));
	QList<QAction*> existingActions(_mapsActionGroup->actions());

	return loadMapNode(maps, action, silent, existingActions);
}

void GUI::mapLoaded()
{
	MapAction *action = static_cast<MapAction*>(QObject::sender());
	Map *map = action->data().value<Map*>();

	if (map->isValid()) {
		action->trigger();
		_showMapAction->setEnabled(true);
		_clearMapCacheAction->setEnabled(true);
	} else {
		QString error = tr("Error loading map:") + "\n\n" + map->path() + "\n\n"
		  + map->errorString();
		QMessageBox::critical(this, APP_NAME, error);
		action->deleteLater();
	}
}

void GUI::mapLoadedDir()
{
	MapAction *action = static_cast<MapAction*>(QObject::sender());
	Map *map = action->data().value<Map*>();

	if (map->isValid()) {
		_showMapAction->setEnabled(true);
		_clearMapCacheAction->setEnabled(true);
		QList<MapAction*> actions;
		actions.append(action);
		_mapView->loadMaps(actions);
	} else {
		QString error = tr("Error loading map:") + "\n\n" + map->path() + "\n\n"
		  + map->errorString();
		QMessageBox::critical(this, APP_NAME, error);
		action->deleteLater();
	}
}

void GUI::loadMapDirNode(const TreeNode<Map *> &node, QList<MapAction*> &actions,
  QMenu *menu, const QList<QAction*> &existingActions)
{
	for (int i = 0; i < node.childs().size(); i++) {
		QMenu *cm = new QMenu(node.childs().at(i).name(), menu);
		menu->addMenu(cm);
		loadMapDirNode(node.childs().at(i), actions, cm, existingActions);
	}

	for (int i = 0; i < node.items().size(); i++) {
		Map *map = node.items().at(i);
		MapAction *a;

		if (!(a = findMapAction(existingActions, map))) {
			if (!map->isValid()) {
				QMessageBox::critical(this, APP_NAME, tr("Error loading map:")
				  + "\n\n" + map->path() + "\n\n" + map->errorString());
				delete map;
			} else {
				a = new MapAction(map, _mapsActionGroup);
				menu->addAction(a);

				if (map->isReady()) {
					_showMapAction->setEnabled(true);
					_clearMapCacheAction->setEnabled(true);
					actions.append(a);
				} else
					connect(a, &MapAction::loaded, this, &GUI::mapLoadedDir);

				_areaCount++;
			}
		} else {
			map = a->data().value<Map*>();
			if (map->isReady())
				actions.append(a);
		}
	}
}

void GUI::loadMapDir()
{
	QString dir(QFileDialog::getExistingDirectory(this,
	  tr("Select map directory"), _mapDir, QFileDialog::ShowDirsOnly));
	if (dir.isEmpty())
		return;

	QFileInfo fi(dir);
	TreeNode<Map*> maps(MapList::loadMaps(dir,
	  CRS::projection(_options.inputProjection)));
	QList<QAction*> existingActions(_mapsActionGroup->actions());
	QList<MapAction*> actions;
	QMenu *menu = new QMenu(maps.name());

	loadMapDirNode(maps, actions, menu, existingActions);

	_mapView->loadMaps(actions);

	if (menu->isEmpty())
		delete menu;
	else
		_mapMenu->insertMenu(_mapsEnd, menu);

	_mapDir = fi.absolutePath();
	_fileActionGroup->setEnabled(true);
	_reloadFileAction->setEnabled(false);
}

void GUI::clearMapCache()
{
	if (QMessageBox::question(this, APP_NAME,
	  tr("Clear the map tile cache?")) == QMessageBox::Yes)
		_mapView->clearMapCache();
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

void GUI::mapChanged(QAction *action)
{
	_map = action->data().value<Map*>();
	_mapView->setMap(_map);
}

void GUI::nextMap()
{
	QAction *checked = _mapsActionGroup->checkedAction();
	if (!checked)
		return;

	QList<QAction*> maps(_mapsActionGroup->actions());
	for (int i = 1;	i < maps.size(); i++) {
		int next = (maps.indexOf(checked) + i) % maps.count();
		if (maps.at(next)->isEnabled()) {
			maps.at(next)->trigger();
			break;
		}
	}
}

void GUI::prevMap()
{
	QAction *checked = _mapsActionGroup->checkedAction();
	if (!checked)
		return;

	QList<QAction*> maps(_mapsActionGroup->actions());
	for (int i = 1; i < maps.size(); i++) {
		int prev = (maps.indexOf(checked) + maps.count() - i) % maps.count();
		if (maps.at(prev)->isEnabled()) {
			maps.at(prev)->trigger();
			break;
		}
	}
}

void GUI::poiFileChecked(QAction *action)
{
	_poi->enableFile(action->data().value<QString>(), action->isChecked());
}

void GUI::selectAllPOIs()
{
	QList<QAction*> actions(_poisActionGroup->actions());
	for (int i = 0; i < actions.size(); i++) {
		POIAction *a = static_cast<POIAction*>(actions.at(i));
		if (_poi->enableFile(a->data().toString(), true))
			a->setChecked(true);
	}
}

void GUI::unselectAllPOIs()
{
	QList<QAction*> actions(_poisActionGroup->actions());
	for (int i = 0; i < actions.size(); i++) {
		POIAction *a = static_cast<POIAction*>(actions.at(i));
		if (_poi->enableFile(a->data().toString(), false))
			a->setChecked(false);
	}
}

void GUI::graphChanged(int index)
{
	if (index < 0)
		return;

	GraphTab *gt = static_cast<GraphTab*>(_graphTabWidget->widget(index));

	_mapView->setGraph(_tabs.indexOf(gt));

	if (_lastTab)
		gt->setSliderPosition(_lastTab->sliderPosition());
	_lastTab = gt;
}

void GUI::updateNavigationActions()
{
	_lastAction->setEnabled(!_browser->isLast());
	_nextAction->setEnabled(!_browser->isLast());
	_firstAction->setEnabled(!_browser->isFirst());
	_prevAction->setEnabled(!_browser->isFirst());
}

bool GUI::updateGraphTabs()
{
	int index;
	GraphTab *tab;
	bool hidden = _graphTabWidget->isHidden();

	for (int i = 0; i < _tabs.size(); i++) {
		tab = _tabs.at(i);
		if (tab->isEmpty() && (index = _graphTabWidget->indexOf(tab)) >= 0)
			_graphTabWidget->removeTab(index);
	}

	for (int i = 0; i < _tabs.size(); i++) {
		tab = _tabs.at(i);
		if (!tab->isEmpty() && _graphTabWidget->indexOf(tab) < 0)
			_graphTabWidget->insertTab(i, tab, _tabs.at(i)->label());
	}

	if (_graphTabWidget->count() &&
	  ((_showTracksAction->isChecked() && _trackCount)
	  || (_showRoutesAction->isChecked() && _routeCount))) {
		if (_showGraphsAction->isChecked())
			_graphTabWidget->setHidden(false);
		_showGraphsAction->setEnabled(true);
	} else {
		_graphTabWidget->setHidden(true);
		_showGraphsAction->setEnabled(false);
	}

	return (hidden != _graphTabWidget->isHidden());
}

void GUI::setTimeType(TimeType type)
{
	for (int i = 0; i <_tabs.count(); i++)
		_tabs.at(i)->setTimeType(type);

	updateStatusBarInfo();
}

void GUI::setUnits(Units units)
{
	_units = units;

	_mapView->setUnits(units);
	for (int i = 0; i <_tabs.count(); i++)
		_tabs.at(i)->setUnits(units);
	updateStatusBarInfo();
}

void GUI::setCoordinatesFormat(CoordinatesFormat format)
{
	_mapView->setCoordinatesFormat(format);
}

void GUI::setGraphType(GraphType type)
{
	for (int i = 0; i <_tabs.count(); i++)
		_tabs.at(i)->setGraphType(type);
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

		case TOGGLE_GRAPH_TYPE_KEY:
			if (_timeGraphAction->isChecked())
				_distanceGraphAction->trigger();
			else
				_timeGraphAction->trigger();
			break;
		case TOGGLE_TIME_TYPE_KEY:
			if (_movingTimeAction->isChecked())
				_totalTimeAction->trigger();
			else
				_movingTimeAction->trigger();
			break;
		case TOGGLE_MARKER_INFO_KEY:
			if (_showMarkerDateAction->isChecked())
				_showMarkerCoordinatesAction->trigger();
			else if (_showMarkerCoordinatesAction->isChecked())
				_showMarkerDateAction->trigger();
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
	MapAction *lastReady = 0;
	QList<QUrl> urls(event->mimeData()->urls());

	for (int i = 0; i < urls.size(); i++) {
		QString file(urls.at(i).toLocalFile());

		if (!openFile(file, true)) {
			MapAction *a;
			if (!loadMap(file, a, true))
				openFile(file, false);
			else {
				if (a)
					lastReady = a;
			}
		}
	}

	if (lastReady)
		lastReady->trigger();

	event->acceptProposedAction();
}

void GUI::writeSettings()
{
	QSettings settings(qApp->applicationName(), qApp->applicationName());
	settings.clear();

	settings.beginGroup(WINDOW_SETTINGS_GROUP);
	if (!_windowStates.isEmpty() && !_windowGeometries.isEmpty()) {
		settings.setValue(WINDOW_STATE_SETTING, _windowStates.first());
		settings.setValue(WINDOW_GEOMETRY_SETTING, _windowGeometries.first());
	} else {
		settings.setValue(WINDOW_STATE_SETTING, saveState());
		settings.setValue(WINDOW_GEOMETRY_SETTING, saveGeometry());
	}
	settings.endGroup();

	settings.beginGroup(SETTINGS_SETTINGS_GROUP);
	if ((_movingTimeAction->isChecked() ? Moving : Total) !=
	  TIME_TYPE_DEFAULT)
		settings.setValue(TIME_TYPE_SETTING, _movingTimeAction->isChecked()
		  ? Moving : Total);
	Units units = _imperialUnitsAction->isChecked() ? Imperial
	  : _nauticalUnitsAction->isChecked() ? Nautical : Metric;
	if (units != UNITS_DEFAULT)
		settings.setValue(UNITS_SETTING, units);
	CoordinatesFormat format = _dmsAction->isChecked() ? DMS
	  : _degreesMinutesAction->isChecked() ? DegreesMinutes : DecimalDegrees;
	if (format != COORDINATES_DEFAULT)
		settings.setValue(COORDINATES_SETTING, format);
	if (_showToolbarsAction->isChecked() != SHOW_TOOLBARS_DEFAULT)
		settings.setValue(SHOW_TOOLBARS_SETTING,
		  _showToolbarsAction->isChecked());
	settings.endGroup();

	settings.beginGroup(MAP_SETTINGS_GROUP);
	settings.setValue(CURRENT_MAP_SETTING, _map->name());
	if (_showMapAction->isChecked() != SHOW_MAP_DEFAULT)
		settings.setValue(SHOW_MAP_SETTING, _showMapAction->isChecked());
	if (_showCoordinatesAction->isChecked() != SHOW_COORDINATES_DEFAULT)
		settings.setValue(SHOW_COORDINATES_SETTING,
		  _showCoordinatesAction->isChecked());
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
	if (_showGraphSliderInfoAction->isChecked()
	  != SHOW_GRAPH_SLIDER_INFO_DEFAULT)
		settings.setValue(SHOW_GRAPH_SLIDER_INFO_SETTING,
		  _showGraphSliderInfoAction->isChecked());
	settings.endGroup();

	settings.beginGroup(POI_SETTINGS_GROUP);
	if (_showPOIAction->isChecked() != SHOW_POI_DEFAULT)
		settings.setValue(SHOW_POI_SETTING, _showPOIAction->isChecked());
	if (_overlapPOIAction->isChecked() != OVERLAP_POI_DEFAULT)
		settings.setValue(OVERLAP_POI_SETTING, _overlapPOIAction->isChecked());
	if (_showPOILabelsAction->isChecked() != SHOW_POI_LABELS_DEFAULT)
		settings.setValue(SHOW_POI_LABELS_SETTING,
		  _showPOILabelsAction->isChecked());

	int j = 0;
	QList<QAction*> poiActions(_poisActionGroup->actions());
	for (int i = 0; i < poiActions.count(); i++) {
		POIAction *a = static_cast<POIAction*>(poiActions.at(i));
		if (!a->isChecked()) {
			if (j == 0)
				settings.beginWriteArray(DISABLED_POI_FILE_SETTINGS_PREFIX);
			settings.setArrayIndex(j++);
			settings.setValue(DISABLED_POI_FILE_SETTING, a->data().toString());
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
	if (_showAreasAction->isChecked() != SHOW_AREAS_DEFAULT)
		settings.setValue(SHOW_AREAS_SETTING, _showAreasAction->isChecked());
	if (_showWaypointLabelsAction->isChecked() != SHOW_WAYPOINT_LABELS_DEFAULT)
		settings.setValue(SHOW_WAYPOINT_LABELS_SETTING,
		  _showWaypointLabelsAction->isChecked());
	if (_showRouteWaypointsAction->isChecked() != SHOW_ROUTE_WAYPOINTS_DEFAULT)
		settings.setValue(SHOW_ROUTE_WAYPOINTS_SETTING,
		  _showRouteWaypointsAction->isChecked());
	if (_showTicksAction->isChecked() != SHOW_TICKS_DEFAULT)
		settings.setValue(SHOW_TICKS_SETTING,
		  _showTicksAction->isChecked());
	bool sm = _showMarkersAction->isChecked()
	  | _showMarkerDateAction->isChecked()
	  | _showMarkerCoordinatesAction->isChecked();
	if (sm != SHOW_MARKERS_DEFAULT)
		settings.setValue(SHOW_MARKERS_SETTING, sm);
	if (_showMarkerDateAction->isChecked()
	  && SHOW_MARKER_INFO_DEFAULT != MarkerInfoItem::Date)
		settings.setValue(SHOW_MARKER_INFO_SETTING, MarkerInfoItem::Date);
	else if (_showMarkerCoordinatesAction->isChecked()
	  && SHOW_MARKER_INFO_DEFAULT != MarkerInfoItem::Position)
		settings.setValue(SHOW_MARKER_INFO_SETTING, MarkerInfoItem::Position);
	settings.endGroup();

	settings.beginGroup(PDF_EXPORT_SETTINGS_GROUP);
	if (_pdfExport.orientation != PAPER_ORIENTATION_DEFAULT)
		settings.setValue(PAPER_ORIENTATION_SETTING, _pdfExport.orientation);
	if (_pdfExport.resolution != RESOLUTION_DEFAULT)
		settings.setValue(RESOLUTION_SETTING, _pdfExport.resolution);
	if (_pdfExport.paperSize != PAPER_SIZE_DEFAULT)
		settings.setValue(PAPER_SIZE_SETTING, _pdfExport.paperSize);
	if (_pdfExport.margins.left() != PDF_MARGIN_LEFT_DEFAULT)
		settings.setValue(PDF_MARGIN_LEFT_SETTING, _pdfExport.margins.left());
	if (_pdfExport.margins.top() != PDF_MARGIN_TOP_DEFAULT)
		settings.setValue(PDF_MARGIN_TOP_SETTING, _pdfExport.margins.top());
	if (_pdfExport.margins.right() != PDF_MARGIN_RIGHT_DEFAULT)
		settings.setValue(PDF_MARGIN_RIGHT_SETTING, _pdfExport.margins.right());
	if (_pdfExport.margins.bottom() != PDF_MARGIN_BOTTOM_DEFAULT)
		settings.setValue(PDF_MARGIN_BOTTOM_SETTING, _pdfExport.margins.bottom());
	if (_pdfExport.fileName != PDF_FILENAME_DEFAULT)
		settings.setValue(PDF_FILENAME_SETTING, _pdfExport.fileName);
	settings.endGroup();

	settings.beginGroup(PNG_EXPORT_SETTINGS_GROUP);
	if (_pngExport.size.width() != PNG_WIDTH_DEFAULT)
		settings.setValue(PNG_WIDTH_SETTING, _pngExport.size.width());
	if (_pngExport.size.height() != PNG_HEIGHT_DEFAULT)
		settings.setValue(PNG_HEIGHT_SETTING, _pngExport.size.height());
	if (_pngExport.margins.left() != PNG_MARGIN_LEFT_DEFAULT)
		settings.setValue(PNG_MARGIN_LEFT_SETTING, _pngExport.margins.left());
	if (_pngExport.margins.top() != PNG_MARGIN_TOP_DEFAULT)
		settings.setValue(PNG_MARGIN_TOP_SETTING, _pngExport.margins.top());
	if (_pngExport.margins.right() != PNG_MARGIN_RIGHT_DEFAULT)
		settings.setValue(PNG_MARGIN_RIGHT_SETTING, _pngExport.margins.right());
	if (_pngExport.margins.bottom() != PNG_MARGIN_BOTTOM_DEFAULT)
		settings.setValue(PNG_MARGIN_BOTTOM_SETTING, _pngExport.margins.bottom());
	if (_pngExport.antialiasing != PNG_ANTIALIASING_DEFAULT)
		settings.setValue(PNG_ANTIALIASING_SETTING, _pngExport.antialiasing);
	if (_pngExport.fileName != PNG_FILENAME_DEFAULT)
		settings.setValue(PNG_FILENAME_SETTING, _pngExport.fileName);
	settings.endGroup();

	settings.beginGroup(OPTIONS_SETTINGS_GROUP);
	if (_options.palette.color() != PALETTE_COLOR_DEFAULT)
		settings.setValue(PALETTE_COLOR_SETTING, _options.palette.color());
	if (_options.palette.shift() != PALETTE_SHIFT_DEFAULT)
		settings.setValue(PALETTE_SHIFT_SETTING, _options.palette.shift());
	if (_options.mapOpacity != MAP_OPACITY_DEFAULT)
		settings.setValue(MAP_OPACITY_SETTING, _options.mapOpacity);
	if (_options.backgroundColor != BACKGROUND_COLOR_DEFAULT)
		settings.setValue(BACKGROUND_COLOR_SETTING, _options.backgroundColor);
	if (_options.trackWidth != TRACK_WIDTH_DEFAULT)
		settings.setValue(TRACK_WIDTH_SETTING, _options.trackWidth);
	if (_options.routeWidth != ROUTE_WIDTH_DEFAULT)
		settings.setValue(ROUTE_WIDTH_SETTING, _options.routeWidth);
	if (_options.areaWidth != AREA_WIDTH_DEFAULT)
		settings.setValue(AREA_WIDTH_SETTING, _options.areaWidth);
	if (_options.trackStyle != TRACK_STYLE_DEFAULT)
		settings.setValue(TRACK_STYLE_SETTING, (int)_options.trackStyle);
	if (_options.routeStyle != ROUTE_STYLE_DEFAULT)
		settings.setValue(ROUTE_STYLE_SETTING, (int)_options.routeStyle);
	if (_options.areaStyle != AREA_STYLE_DEFAULT)
		settings.setValue(AREA_STYLE_SETTING, (int)_options.areaStyle);
	if (_options.areaOpacity != AREA_OPACITY_DEFAULT)
		settings.setValue(AREA_OPACITY_SETTING, (int)_options.areaOpacity);
	if (_options.waypointSize != WAYPOINT_SIZE_DEFAULT)
		settings.setValue(WAYPOINT_SIZE_SETTING, _options.waypointSize);
	if (_options.waypointColor != WAYPOINT_COLOR_DEFAULT)
		settings.setValue(WAYPOINT_COLOR_SETTING, _options.waypointColor);
	if (_options.poiSize != POI_SIZE_DEFAULT)
		settings.setValue(POI_SIZE_SETTING, _options.poiSize);
	if (_options.poiColor != POI_COLOR_DEFAULT)
		settings.setValue(POI_COLOR_SETTING, _options.poiColor);
	if (_options.graphWidth != GRAPH_WIDTH_DEFAULT)
		settings.setValue(GRAPH_WIDTH_SETTING, _options.graphWidth);
	if (_options.pathAntiAliasing != PATH_AA_DEFAULT)
		settings.setValue(PATH_AA_SETTING, _options.pathAntiAliasing);
	if (_options.graphAntiAliasing != GRAPH_AA_DEFAULT)
		settings.setValue(GRAPH_AA_SETTING, _options.graphAntiAliasing);
	if (_options.elevationFilter != ELEVATION_FILTER_DEFAULT)
		settings.setValue(ELEVATION_FILTER_SETTING, _options.elevationFilter);
	if (_options.speedFilter != SPEED_FILTER_DEFAULT)
		settings.setValue(SPEED_FILTER_SETTING, _options.speedFilter);
	if (_options.heartRateFilter != HEARTRATE_FILTER_DEFAULT)
		settings.setValue(HEARTRATE_FILTER_SETTING, _options.heartRateFilter);
	if (_options.cadenceFilter != CADENCE_FILTER_DEFAULT)
		settings.setValue(CADENCE_FILTER_SETTING, _options.cadenceFilter);
	if (_options.powerFilter != POWER_FILTER_DEFAULT)
		settings.setValue(POWER_FILTER_SETTING, _options.powerFilter);
	if (_options.outlierEliminate != OUTLIER_ELIMINATE_DEFAULT)
		settings.setValue(OUTLIER_ELIMINATE_SETTING, _options.outlierEliminate);
	if (_options.automaticPause != AUTOMATIC_PAUSE_DEFAULT)
		settings.setValue(AUTOMATIC_PAUSE_SETTING, _options.automaticPause);
	if (_options.pauseSpeed != PAUSE_SPEED_DEFAULT)
		settings.setValue(PAUSE_SPEED_SETTING, _options.pauseSpeed);
	if (_options.pauseInterval != PAUSE_INTERVAL_DEFAULT)
		settings.setValue(PAUSE_INTERVAL_SETTING, _options.pauseInterval);
	if (_options.useReportedSpeed != USE_REPORTED_SPEED_DEFAULT)
		settings.setValue(USE_REPORTED_SPEED_SETTING, _options.useReportedSpeed);
	if (_options.dataUseDEM != DATA_USE_DEM_DEFAULT)
		settings.setValue(DATA_USE_DEM_SETTING, _options.dataUseDEM);
	if (_options.showSecondaryElevation != SHOW_SECONDARY_ELEVATION_DEFAULT)
		settings.setValue(SHOW_SECONDARY_ELEVATION_SETTING,
		  _options.showSecondaryElevation);
	if (_options.showSecondarySpeed != SHOW_SECONDARY_SPEED_DEFAULT)
		settings.setValue(SHOW_SECONDARY_SPEED_SETTING,
		  _options.showSecondarySpeed);
	if (_options.timeZone != TimeZoneInfo())
		settings.setValue(TIME_ZONE_SETTING, QVariant::fromValue(
		  _options.timeZone));
	if (_options.useSegments != USE_SEGMENTS_DEFAULT)
		settings.setValue(USE_SEGMENTS_SETTING, _options.useSegments);
	if (_options.poiRadius != POI_RADIUS_DEFAULT)
		settings.setValue(POI_RADIUS_SETTING, _options.poiRadius);
	if (_options.useOpenGL != USE_OPENGL_DEFAULT)
		settings.setValue(USE_OPENGL_SETTING, _options.useOpenGL);
	if (_options.enableHTTP2 != ENABLE_HTTP2_DEFAULT)
		settings.setValue(ENABLE_HTTP2_SETTING, _options.enableHTTP2);
	if (_options.pixmapCache != PIXMAP_CACHE_DEFAULT)
		settings.setValue(PIXMAP_CACHE_SETTING, _options.pixmapCache);
	if (_options.connectionTimeout != CONNECTION_TIMEOUT_DEFAULT)
		settings.setValue(CONNECTION_TIMEOUT_SETTING, _options.connectionTimeout);
	if (_options.hiresPrint != HIRES_PRINT_DEFAULT)
		settings.setValue(HIRES_PRINT_SETTING, _options.hiresPrint);
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
	if (_options.sliderColor != SLIDER_COLOR_DEFAULT)
		settings.setValue(SLIDER_COLOR_SETTING, _options.sliderColor);
	if (_options.outputProjection != OUTPUT_PROJECTION_DEFAULT)
		settings.setValue(OUTPUT_PROJECTION_SETTING, _options.outputProjection);
	if (_options.inputProjection != INPUT_PROJECTION_DEFAULT)
		settings.setValue(INPUT_PROJECTION_SETTING, _options.inputProjection);
	if (_options.hidpiMap != HIDPI_MAP_DEFAULT)
		settings.setValue(HIDPI_MAP_SETTING, _options.hidpiMap);
	if (_options.dataPath != DATA_PATH_DEFAULT)
		settings.setValue(DATA_PATH_SETTING, _options.dataPath);
	if (_options.mapsPath != MAPS_PATH_DEFAULT)
		settings.setValue(MAPS_PATH_SETTING, _options.mapsPath);
	if (_options.poiPath != POI_PATH_DEFAULT)
		settings.setValue(POI_PATH_SETTING, _options.poiPath);

	settings.endGroup();
}

void GUI::readSettings()
{
	int value;
	QSettings settings(qApp->applicationName(), qApp->applicationName());

	settings.beginGroup(WINDOW_SETTINGS_GROUP);
	restoreGeometry(settings.value(WINDOW_GEOMETRY_SETTING).toByteArray());
	restoreState(settings.value(WINDOW_STATE_SETTING).toByteArray());
	settings.endGroup();

	settings.beginGroup(SETTINGS_SETTINGS_GROUP);
	if (settings.value(TIME_TYPE_SETTING, TIME_TYPE_DEFAULT).toInt() == Moving)
		_movingTimeAction->trigger();
	else
		_totalTimeAction->trigger();

	value = settings.value(UNITS_SETTING, UNITS_DEFAULT).toInt();
	if (value == Imperial)
		_imperialUnitsAction->trigger();
	else if (value == Nautical)
		_nauticalUnitsAction->trigger();
	else
		_metricUnitsAction->trigger();

	value = settings.value(COORDINATES_SETTING, COORDINATES_DEFAULT).toInt();
	if (value == DMS)
		_dmsAction->trigger();
	else if (value == DegreesMinutes)
		_degreesMinutesAction->trigger();
	else
		_decimalDegreesAction->trigger();

	if (!settings.value(SHOW_TOOLBARS_SETTING, SHOW_TOOLBARS_DEFAULT).toBool())
		showToolbars(false);
	else
		_showToolbarsAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(MAP_SETTINGS_GROUP);
	if (settings.value(SHOW_MAP_SETTING, SHOW_MAP_DEFAULT).toBool())
		_showMapAction->setChecked(true);
	else
		_mapView->showMap(false);
	QAction *ma = mapAction(settings.value(CURRENT_MAP_SETTING).toString());
	if (ma) {
		ma->trigger();
		_showMapAction->setEnabled(true);
		_clearMapCacheAction->setEnabled(true);
	}
	if (settings.value(SHOW_COORDINATES_SETTING, SHOW_COORDINATES_DEFAULT)
	  .toBool()) {
		_showCoordinatesAction->setChecked(true);
		_mapView->showCoordinates(true);
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
	if (!settings.value(SHOW_GRAPH_SLIDER_INFO_SETTING,
	  SHOW_GRAPH_SLIDER_INFO_DEFAULT).toBool())
		showGraphSliderInfo(false);
	else
		_showGraphSliderInfoAction->setChecked(true);
	settings.endGroup();

	settings.beginGroup(POI_SETTINGS_GROUP);
	if (!settings.value(OVERLAP_POI_SETTING, OVERLAP_POI_DEFAULT).toBool())
		_mapView->showOverlappedPOIs(false);
	else
		_overlapPOIAction->setChecked(true);
	if (!settings.value(SHOW_POI_LABELS_SETTING, SHOW_POI_LABELS_DEFAULT)
	  .toBool())
		_mapView->showPOILabels(false);
	else
		_showPOILabelsAction->setChecked(true);
	if (settings.value(SHOW_POI_SETTING, SHOW_POI_DEFAULT).toBool())
		_showPOIAction->setChecked(true);
	else
		_mapView->showPOI(false);
	QList<QAction*> poiActions(_poisActionGroup->actions());
	for (int i = 0; i < poiActions.count(); i++)
		poiActions.at(i)->setChecked(true);
	int size = settings.beginReadArray(DISABLED_POI_FILE_SETTINGS_PREFIX);
	for (int i = 0; i < size; i++) {
		settings.setArrayIndex(i);
		QString file(settings.value(DISABLED_POI_FILE_SETTING).toString());
		if (_poi->enableFile(file, false)) {
			for (int j = 0; j < poiActions.size(); j++)
				if (poiActions.at(j)->data().toString() == file)
					poiActions.at(j)->setChecked(false);
		}
	}
	settings.endArray();
	settings.endGroup();

	settings.beginGroup(DATA_SETTINGS_GROUP);
	if (!settings.value(SHOW_TRACKS_SETTING, SHOW_TRACKS_DEFAULT).toBool()) {
		_mapView->showTracks(false);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showTracks(false);
	} else
		_showTracksAction->setChecked(true);
	if (!settings.value(SHOW_ROUTES_SETTING, SHOW_ROUTES_DEFAULT).toBool()) {
		_mapView->showRoutes(false);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showRoutes(false);
	} else
		_showRoutesAction->setChecked(true);
	if (!settings.value(SHOW_WAYPOINTS_SETTING, SHOW_WAYPOINTS_DEFAULT)
	  .toBool())
		_mapView->showWaypoints(false);
	else
		_showWaypointsAction->setChecked(true);
	if (!settings.value(SHOW_AREAS_SETTING, SHOW_AREAS_DEFAULT).toBool())
		_mapView->showAreas(false);
	else
		_showAreasAction->setChecked(true);
	if (!settings.value(SHOW_WAYPOINT_LABELS_SETTING,
	  SHOW_WAYPOINT_LABELS_DEFAULT).toBool())
		_mapView->showWaypointLabels(false);
	else
		_showWaypointLabelsAction->setChecked(true);
	if (!settings.value(SHOW_ROUTE_WAYPOINTS_SETTING,
	  SHOW_ROUTE_WAYPOINTS_SETTING).toBool())
		_mapView->showRouteWaypoints(false);
	else
		_showRouteWaypointsAction->setChecked(true);
	if (settings.value(SHOW_TICKS_SETTING, SHOW_TICKS_DEFAULT).toBool()) {
		_mapView->showTicks(true);
		_showTicksAction->setChecked(true);
	}
	if (settings.value(SHOW_MARKERS_SETTING, SHOW_MARKERS_DEFAULT).toBool()) {
		MarkerInfoItem::Type mt = static_cast<MarkerInfoItem::Type>
		  (settings.value(SHOW_MARKER_INFO_SETTING,
		  SHOW_MARKER_INFO_DEFAULT).toInt());
		if (mt == MarkerInfoItem::Position)
			_showMarkerCoordinatesAction->trigger();
		else if (mt == MarkerInfoItem::Date)
			_showMarkerDateAction->trigger();
		else
			_showMarkersAction->trigger();
	} else
		_hideMarkersAction->trigger();
	settings.endGroup();

	settings.beginGroup(PDF_EXPORT_SETTINGS_GROUP);
	_pdfExport.orientation = (QPageLayout::Orientation) settings.value(
	  PAPER_ORIENTATION_SETTING, PAPER_ORIENTATION_DEFAULT).toInt();
	_pdfExport.resolution = settings.value(RESOLUTION_SETTING,
	  RESOLUTION_DEFAULT).toInt();
	_pdfExport.paperSize = (QPageSize::PageSizeId) settings.value(
	  PAPER_SIZE_SETTING, PAPER_SIZE_DEFAULT).toInt();
	qreal ml = settings.value(PDF_MARGIN_LEFT_SETTING, PDF_MARGIN_LEFT_DEFAULT)
	  .toReal();
	qreal mt = settings.value(PDF_MARGIN_TOP_SETTING, PDF_MARGIN_TOP_DEFAULT)
	  .toReal();
	qreal mr = settings.value(PDF_MARGIN_RIGHT_SETTING,
	  PDF_MARGIN_RIGHT_DEFAULT).toReal();
	qreal mb = settings.value(PDF_MARGIN_BOTTOM_SETTING,
	  PDF_MARGIN_BOTTOM_DEFAULT).toReal();
	_pdfExport.margins = QMarginsF(ml, mt, mr, mb);
	_pdfExport.fileName = settings.value(PDF_FILENAME_SETTING,
	 PDF_FILENAME_DEFAULT).toString();
	settings.endGroup();

	settings.beginGroup(PNG_EXPORT_SETTINGS_GROUP);
	_pngExport.size = QSize(settings.value(PNG_WIDTH_SETTING, PNG_WIDTH_DEFAULT)
	  .toInt(), settings.value(PNG_HEIGHT_SETTING, PNG_HEIGHT_DEFAULT).toInt());
	int mli = settings.value(PNG_MARGIN_LEFT_SETTING, PNG_MARGIN_LEFT_DEFAULT)
	  .toInt();
	int mti = settings.value(PNG_MARGIN_TOP_SETTING, PNG_MARGIN_TOP_DEFAULT)
	  .toInt();
	int mri = settings.value(PNG_MARGIN_RIGHT_SETTING, PNG_MARGIN_RIGHT_DEFAULT)
	  .toInt();
	int mbi = settings.value(PNG_MARGIN_BOTTOM_SETTING,
	  PNG_MARGIN_BOTTOM_DEFAULT).toInt();
	_pngExport.margins = QMargins(mli, mti, mri, mbi);
	_pngExport.antialiasing = settings.value(PNG_ANTIALIASING_SETTING,
	  PNG_ANTIALIASING_DEFAULT).toBool();
	_pngExport.fileName = settings.value(PNG_FILENAME_SETTING,
	  PNG_FILENAME_DEFAULT).toString();
	settings.endGroup();

	settings.beginGroup(OPTIONS_SETTINGS_GROUP);
	QColor pc = settings.value(PALETTE_COLOR_SETTING, PALETTE_COLOR_DEFAULT)
	  .value<QColor>();
	qreal ps = settings.value(PALETTE_SHIFT_SETTING, PALETTE_SHIFT_DEFAULT)
	  .toDouble();
	_options.palette = Palette(pc, ps);
	_options.mapOpacity = settings.value(MAP_OPACITY_SETTING,
	  MAP_OPACITY_DEFAULT).toInt();
	_options.backgroundColor = settings.value(BACKGROUND_COLOR_SETTING,
	  BACKGROUND_COLOR_DEFAULT).value<QColor>();
	_options.trackWidth = settings.value(TRACK_WIDTH_SETTING,
	  TRACK_WIDTH_DEFAULT).toInt();
	_options.routeWidth = settings.value(ROUTE_WIDTH_SETTING,
	  ROUTE_WIDTH_DEFAULT).toInt();
	_options.areaWidth = settings.value(AREA_WIDTH_SETTING,
	  AREA_WIDTH_DEFAULT).toInt();
	_options.trackStyle = (Qt::PenStyle) settings.value(TRACK_STYLE_SETTING,
	  (int)TRACK_STYLE_DEFAULT).toInt();
	_options.routeStyle = (Qt::PenStyle) settings.value(ROUTE_STYLE_SETTING,
	  (int)ROUTE_STYLE_DEFAULT).toInt();
	_options.areaStyle = (Qt::PenStyle) settings.value(AREA_STYLE_SETTING,
	  (int)AREA_STYLE_DEFAULT).toInt();
	_options.areaOpacity = settings.value(AREA_OPACITY_SETTING,
	  AREA_OPACITY_DEFAULT).toInt();
	_options.pathAntiAliasing = settings.value(PATH_AA_SETTING, PATH_AA_DEFAULT)
	  .toBool();
	_options.waypointSize = settings.value(WAYPOINT_SIZE_SETTING,
	  WAYPOINT_SIZE_DEFAULT).toInt();
	_options.waypointColor = settings.value(WAYPOINT_COLOR_SETTING,
	  WAYPOINT_COLOR_DEFAULT).value<QColor>();
	_options.poiSize = settings.value(POI_SIZE_SETTING, POI_SIZE_DEFAULT)
	  .toInt();
	_options.poiColor = settings.value(POI_COLOR_SETTING, POI_COLOR_DEFAULT)
	  .value<QColor>();
	_options.graphWidth = settings.value(GRAPH_WIDTH_SETTING,
	  GRAPH_WIDTH_DEFAULT).toInt();
	_options.graphAntiAliasing = settings.value(GRAPH_AA_SETTING,
	  GRAPH_AA_DEFAULT).toBool();
	_options.elevationFilter = settings.value(ELEVATION_FILTER_SETTING,
	  ELEVATION_FILTER_DEFAULT).toInt();
	_options.speedFilter = settings.value(SPEED_FILTER_SETTING,
	  SPEED_FILTER_DEFAULT).toInt();
	_options.heartRateFilter = settings.value(HEARTRATE_FILTER_SETTING,
	  HEARTRATE_FILTER_DEFAULT).toInt();
	_options.cadenceFilter = settings.value(CADENCE_FILTER_SETTING,
	  CADENCE_FILTER_DEFAULT).toInt();
	_options.powerFilter = settings.value(POWER_FILTER_SETTING,
	  POWER_FILTER_DEFAULT).toInt();
	_options.outlierEliminate = settings.value(OUTLIER_ELIMINATE_SETTING,
	  OUTLIER_ELIMINATE_DEFAULT).toBool();
	_options.pauseSpeed = settings.value(PAUSE_SPEED_SETTING,
	  PAUSE_SPEED_DEFAULT).toFloat();
	_options.useReportedSpeed = settings.value(USE_REPORTED_SPEED_SETTING,
	  USE_REPORTED_SPEED_DEFAULT).toBool();
	_options.dataUseDEM = settings.value(DATA_USE_DEM_SETTING,
	  DATA_USE_DEM_DEFAULT).toBool();
	_options.showSecondaryElevation = settings.value(
	  SHOW_SECONDARY_ELEVATION_SETTING,
	  SHOW_SECONDARY_ELEVATION_DEFAULT).toBool();
	_options.showSecondarySpeed = settings.value(
	  SHOW_SECONDARY_SPEED_SETTING,
	  SHOW_SECONDARY_SPEED_DEFAULT).toBool();
	_options.timeZone = settings.value(TIME_ZONE_SETTING).value<TimeZoneInfo>();
	_options.useSegments = settings.value(USE_SEGMENTS_SETTING,
	  USE_SEGMENTS_DEFAULT).toBool();
	_options.automaticPause = settings.value(AUTOMATIC_PAUSE_SETTING,
	  AUTOMATIC_PAUSE_DEFAULT).toBool();
	_options.pauseInterval = settings.value(PAUSE_INTERVAL_SETTING,
	  PAUSE_INTERVAL_DEFAULT).toInt();
	_options.poiRadius = settings.value(POI_RADIUS_SETTING, POI_RADIUS_DEFAULT)
	  .toInt();
	_options.useOpenGL = settings.value(USE_OPENGL_SETTING, USE_OPENGL_DEFAULT)
	  .toBool();
	_options.enableHTTP2 = settings.value(ENABLE_HTTP2_SETTING,
	  ENABLE_HTTP2_DEFAULT).toBool();
	_options.pixmapCache = settings.value(PIXMAP_CACHE_SETTING,
	  PIXMAP_CACHE_DEFAULT).toInt();
	_options.connectionTimeout = settings.value(CONNECTION_TIMEOUT_SETTING,
	  CONNECTION_TIMEOUT_DEFAULT).toInt();
	_options.hiresPrint = settings.value(HIRES_PRINT_SETTING,
	  HIRES_PRINT_DEFAULT).toBool();
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
	_options.sliderColor = settings.value(SLIDER_COLOR_SETTING,
	  SLIDER_COLOR_DEFAULT).value<QColor>();
	_options.outputProjection = settings.value(OUTPUT_PROJECTION_SETTING,
	  OUTPUT_PROJECTION_DEFAULT).toInt();
	_options.inputProjection = settings.value(INPUT_PROJECTION_SETTING,
	  INPUT_PROJECTION_DEFAULT).toInt();
	_options.hidpiMap = settings.value(HIDPI_MAP_SETTING, HIDPI_MAP_DEFAULT)
	  .toBool();
	_options.dataPath = settings.value(DATA_PATH_SETTING, DATA_PATH_DEFAULT)
	  .toString();
	_options.mapsPath = settings.value(MAPS_PATH_SETTING, MAPS_PATH_DEFAULT)
	  .toString();
	_options.poiPath = settings.value(POI_PATH_SETTING, POI_PATH_DEFAULT)
	  .toString();

	_mapView->setPalette(_options.palette);
	_mapView->setMapOpacity(_options.mapOpacity);
	_mapView->setBackgroundColor(_options.backgroundColor);
	_mapView->setTrackWidth(_options.trackWidth);
	_mapView->setRouteWidth(_options.routeWidth);
	_mapView->setAreaWidth(_options.areaWidth);
	_mapView->setTrackStyle(_options.trackStyle);
	_mapView->setRouteStyle(_options.routeStyle);
	_mapView->setAreaStyle(_options.areaStyle);
	_mapView->setAreaOpacity(_options.areaOpacity);
	_mapView->setWaypointSize(_options.waypointSize);
	_mapView->setWaypointColor(_options.waypointColor);
	_mapView->setPOISize(_options.poiSize);
	_mapView->setPOIColor(_options.poiColor);
	_mapView->setRenderHint(QPainter::Antialiasing, _options.pathAntiAliasing);
	_mapView->setMarkerColor(_options.sliderColor);
	if (_options.useOpenGL)
		_mapView->useOpenGL(true);
	_mapView->setDevicePixelRatio(devicePixelRatioF(),
	  _options.hidpiMap ? devicePixelRatioF() : 1.0);
	_mapView->setOutputProjection(CRS::projection(_options.outputProjection));
	_mapView->setInputProjection(CRS::projection(_options.inputProjection));
	_mapView->setTimeZone(_options.timeZone.zone());

	for (int i = 0; i < _tabs.count(); i++) {
		_tabs.at(i)->setPalette(_options.palette);
		_tabs.at(i)->setGraphWidth(_options.graphWidth);
		_tabs.at(i)->setRenderHint(QPainter::Antialiasing,
		  _options.graphAntiAliasing);
		_tabs.at(i)->setSliderColor(_options.sliderColor);
		if (_options.useOpenGL)
			_tabs.at(i)->useOpenGL(true);
	}

	Track::setElevationFilter(_options.elevationFilter);
	Track::setSpeedFilter(_options.speedFilter);
	Track::setHeartRateFilter(_options.heartRateFilter);
	Track::setCadenceFilter(_options.cadenceFilter);
	Track::setPowerFilter(_options.powerFilter);
	Track::setOutlierElimination(_options.outlierEliminate);
	Track::setAutomaticPause(_options.automaticPause);
	Track::setPauseSpeed(_options.pauseSpeed);
	Track::setPauseInterval(_options.pauseInterval);
	Track::useReportedSpeed(_options.useReportedSpeed);
	Track::useDEM(_options.dataUseDEM);
	Track::showSecondaryElevation(_options.showSecondaryElevation);
	Track::showSecondarySpeed(_options.showSecondarySpeed);
	Track::useSegments(_options.useSegments);
	Route::useDEM(_options.dataUseDEM);
	Route::showSecondaryElevation(_options.showSecondaryElevation);
	Waypoint::useDEM(_options.dataUseDEM);
	Waypoint::showSecondaryElevation(_options.showSecondaryElevation);

	_poi->setRadius(_options.poiRadius);

	QPixmapCache::setCacheLimit(_options.pixmapCache * 1024);

	settings.endGroup();
}

QAction *GUI::mapAction(const QString &name)
{
	QList<QAction *> maps(_mapsActionGroup->actions());

	// Last map
	for (int i = 0; i < maps.count(); i++) {
		Map *map = maps.at(i)->data().value<Map*>();
		if (map->name() == name && map->isReady())
			return maps.at(i);
	}

	// Any usable map
	for (int i = 0; i < maps.count(); i++) {
		Map *map = maps.at(i)->data().value<Map*>();
		if (map->isReady())
			return maps.at(i);
	}

	return 0;
}

Units GUI::units() const
{
	return _imperialUnitsAction->isChecked() ? Imperial
	  : _nauticalUnitsAction->isChecked() ? Nautical : Metric;
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

void GUI::show()
{
	QMainWindow::show();

	QWindow *w = windowHandle();
	connect(w->screen(), &QScreen::logicalDotsPerInchChanged, this,
	  &GUI::logicalDotsPerInchChanged);
	connect(w, &QWindow::screenChanged, this, &GUI::screenChanged);

	_mapView->fitContentToSize();
}

void GUI::screenChanged(QScreen *screen)
{
	_mapView->setDevicePixelRatio(devicePixelRatioF(),
	  _options.hidpiMap ? devicePixelRatioF() : 1.0);

	disconnect(SIGNAL(logicalDotsPerInchChanged(qreal)), this,
	  SLOT(logicalDotsPerInchChanged(qreal)));
	connect(screen, &QScreen::logicalDotsPerInchChanged, this,
	  &GUI::logicalDotsPerInchChanged);
}

void GUI::logicalDotsPerInchChanged(qreal dpi)
{
	Q_UNUSED(dpi)

	_mapView->setDevicePixelRatio(devicePixelRatioF(),
	  _options.hidpiMap ? devicePixelRatioF() : 1.0);
}
