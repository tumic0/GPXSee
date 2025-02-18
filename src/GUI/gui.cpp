#include <QApplication>
#include <QSplitter>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QCheckBox>
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
#include <QGeoPositionInfoSource>
#include "common/config.h"
#include "common/programpaths.h"
#include "data/data.h"
#include "data/poi.h"
#include "map/downloader.h"
#include "map/demloader.h"
#include "map/maplist.h"
#include "map/emptymap.h"
#include "map/crs.h"
#include "map/hillshading.h"
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
#include "mapaction.h"
#include "poiaction.h"
#include "gui.h"
#ifdef Q_OS_ANDROID
#include "common/util.h"
#include "navigationwidget.h"
#endif // Q_OS_ANDROID


#define MAX_RECENT_FILES  10
#define TOOLBAR_ICON_SIZE 22

GUI::GUI()
{
	QString activeMap;
	QStringList disabledPOIs, recentFiles;

	_poi = new POI(this);
	_dem = new DEMLoader(ProgramPaths::demDir(true), this);
	connect(_dem, &DEMLoader::finished, this, &GUI::demLoaded);

	createMapView();
	createGraphTabs();
	createStatusBar();
	createActions();
	createMenus();
#ifdef Q_OS_ANDROID
	createNavigation();
#else // Q_OS_ANDROID
	createToolBars();
#endif // Q_OS_ANDROID
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

	readSettings(activeMap, disabledPOIs, recentFiles);

	loadInitialMaps(activeMap);
	loadInitialPOIs(disabledPOIs);
#ifndef Q_OS_ANDROID
	loadRecentFiles(recentFiles);
#endif // Q_OS_ANDROID

	updateGraphTabs();
	updateStatusBarInfo();
	updateMapDEMDownloadAction();
}

void GUI::createBrowser()
{
	_browser = new FileBrowser(this);
	_browser->setFilter(Data::filter());
	connect(_browser, &FileBrowser::listChanged, this,
	  &GUI::updateNavigationActions);
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
		} else
			delete map;
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
		qWarning("%s: %s", qUtf8Printable(map->path()),
		  qUtf8Printable(map->errorString()));
		action->deleteLater();
	}
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
#if !defined(Q_OS_MAC) && !defined(Q_OS_ANDROID)
	_exitAction = new QAction(QIcon::fromTheme(QUIT_NAME, QIcon(QUIT_ICON)),
	  tr("Quit"), this);
	_exitAction->setShortcut(QUIT_SHORTCUT);
	_exitAction->setMenuRole(QAction::QuitRole);
	connect(_exitAction, &QAction::triggered, this, &GUI::close);
	addAction(_exitAction);
#endif // Q_OS_MAC + Q_OS_ANDROID

	// Help & About
	_pathsAction = new QAction(tr("Paths"), this);
	_pathsAction->setMenuRole(QAction::NoRole);
	connect(_pathsAction, &QAction::triggered, this, &GUI::paths);
#ifndef Q_OS_ANDROID
	_keysAction = new QAction(tr("Keyboard controls"), this);
	_keysAction->setMenuRole(QAction::NoRole);
	connect(_keysAction, &QAction::triggered, this, &GUI::keys);
#endif // Q_OS_ANDROID
	_aboutAction = new QAction(QIcon(APP_ICON), tr("About GPXSee"), this);
	_aboutAction->setMenuRole(QAction::AboutRole);
	connect(_aboutAction, &QAction::triggered, this, &GUI::about);

	// File actions
	_openFileAction = new QAction(QIcon::fromTheme(OPEN_FILE_NAME,
	  QIcon(OPEN_FILE_ICON)), tr("Open..."), this);
	_openFileAction->setMenuRole(QAction::NoRole);
	_openFileAction->setShortcut(OPEN_SHORTCUT);
	connect(_openFileAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::openFile));
	addAction(_openFileAction);
	_openDirAction = new QAction(QIcon::fromTheme(OPEN_DIR_NAME,
	  QIcon(OPEN_DIR_ICON)), tr("Open directory..."), this);
	_openDirAction->setMenuRole(QAction::NoRole);
	connect(_openDirAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::openDir));
	_printFileAction = new QAction(QIcon::fromTheme(PRINT_FILE_NAME,
	  QIcon(PRINT_FILE_ICON)), tr("Print..."), this);
	_printFileAction->setMenuRole(QAction::NoRole);
	_printFileAction->setActionGroup(_fileActionGroup);
	connect(_printFileAction, &QAction::triggered, this, &GUI::printFile);
	addAction(_printFileAction);
	_exportPDFFileAction = new QAction(QIcon::fromTheme(EXPORT_FILE_NAME,
	  QIcon(EXPORT_FILE_ICON)), tr("Export to PDF..."), this);
	_exportPDFFileAction->setMenuRole(QAction::NoRole);
	_exportPDFFileAction->setShortcut(PDF_EXPORT_SHORTCUT);
	_exportPDFFileAction->setActionGroup(_fileActionGroup);
	connect(_exportPDFFileAction, &QAction::triggered, this, &GUI::exportPDFFile);
	addAction(_exportPDFFileAction);
	_exportPNGFileAction = new QAction(QIcon::fromTheme(EXPORT_FILE_NAME,
	  QIcon(EXPORT_FILE_ICON)), tr("Export to PNG..."), this);
	_exportPNGFileAction->setMenuRole(QAction::NoRole);
	_exportPNGFileAction->setShortcut(PNG_EXPORT_SHORTCUT);
	_exportPNGFileAction->setActionGroup(_fileActionGroup);
	connect(_exportPNGFileAction, &QAction::triggered, this, &GUI::exportPNGFile);
	addAction(_exportPNGFileAction);
	_closeFileAction = new QAction(QIcon::fromTheme(CLOSE_FILE_NAME,
	  QIcon(CLOSE_FILE_ICON)), tr("Close"), this);
	_closeFileAction->setMenuRole(QAction::NoRole);
	_closeFileAction->setShortcut(CLOSE_SHORTCUT);
	_closeFileAction->setActionGroup(_fileActionGroup);
	connect(_closeFileAction, &QAction::triggered, this, &GUI::closeAll);
	addAction(_closeFileAction);
	_reloadFileAction = new QAction(QIcon::fromTheme(RELOAD_FILE_NAME,
	  QIcon(RELOAD_FILE_ICON)), tr("Reload"), this);
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
#ifndef Q_OS_ANDROID
	_recentFilesActionGroup = new QActionGroup(this);
	connect(_recentFilesActionGroup, &QActionGroup::triggered, this,
	  &GUI::recentFileSelected);
	_clearRecentFilesAction = new QAction(tr("Clear list"), this);
	_clearRecentFilesAction->setMenuRole(QAction::NoRole);
	connect(_clearRecentFilesAction, &QAction::triggered, this,
	  &GUI::clearRecentFiles);
#endif // Q_OS_ANDROID

	// POI actions
	_poisActionGroup = new QActionGroup(this);
	_poisActionGroup->setExclusive(false);
	connect(_poisActionGroup, &QActionGroup::triggered, this,
	  &GUI::poiFileChecked);
	_openPOIAction = new QAction(QIcon::fromTheme(OPEN_FILE_NAME,
	  QIcon(OPEN_FILE_ICON)), tr("Load POI file..."), this);
	_openPOIAction->setMenuRole(QAction::NoRole);
	connect(_openPOIAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::openPOIFile));
	_selectAllPOIAction = new QAction(tr("Select all files"), this);
	_selectAllPOIAction->setMenuRole(QAction::NoRole);
	_selectAllPOIAction->setEnabled(false);
	connect(_selectAllPOIAction, &QAction::triggered, this,
	  &GUI::selectAllPOIs);
	_unselectAllPOIAction = new QAction(tr("Unselect all files"), this);
	_unselectAllPOIAction->setMenuRole(QAction::NoRole);
	_unselectAllPOIAction->setEnabled(false);
	connect(_unselectAllPOIAction, &QAction::triggered, this,
	  &GUI::unselectAllPOIs);
	_overlapPOIAction = new QAction(tr("Overlap POIs"), this);
	_overlapPOIAction->setMenuRole(QAction::NoRole);
	_overlapPOIAction->setCheckable(true);
	connect(_overlapPOIAction, &QAction::triggered, _mapView,
	  &MapView::showOverlappedPOIs);
	_showPOIIconsAction = new QAction(tr("Show POI icons"), this);
	_showPOIIconsAction->setMenuRole(QAction::NoRole);
	_showPOIIconsAction->setCheckable(true);
	connect(_showPOIIconsAction, &QAction::triggered, _mapView,
	  &MapView::showPOIIcons);
	_showPOILabelsAction = new QAction(tr("Show POI labels"), this);
	_showPOILabelsAction->setMenuRole(QAction::NoRole);
	_showPOILabelsAction->setCheckable(true);
	connect(_showPOILabelsAction, &QAction::triggered, _mapView,
	  &MapView::showPOILabels);
	_showPOIAction = new QAction(QIcon::fromTheme(SHOW_POI_NAME,
	  QIcon(SHOW_POI_ICON)), tr("Show POIs"), this);
	_showPOIAction->setMenuRole(QAction::NoRole);
	_showPOIAction->setCheckable(true);
	_showPOIAction->setShortcut(SHOW_POI_SHORTCUT);
	connect(_showPOIAction, &QAction::triggered, _mapView, &MapView::showPOI);
	addAction(_showPOIAction);

	// Map actions
	_mapsActionGroup = new QActionGroup(this);
	_mapsActionGroup->setExclusive(true);
	connect(_mapsActionGroup, &QActionGroup::triggered, this, &GUI::mapChanged);
	_showMapAction = new QAction(QIcon::fromTheme(SHOW_MAP_NAME,
	  QIcon(SHOW_MAP_ICON)), tr("Show map"), this);
	_showMapAction->setEnabled(false);
	_showMapAction->setMenuRole(QAction::NoRole);
	_showMapAction->setCheckable(true);
	_showMapAction->setShortcut(SHOW_MAP_SHORTCUT);
	connect(_showMapAction, &QAction::triggered, _mapView,
	  &MapView::showMap);
	addAction(_showMapAction);
	_loadMapAction = new QAction(QIcon::fromTheme(OPEN_FILE_NAME,
	  QIcon(OPEN_FILE_ICON)), tr("Load map..."), this);
	_loadMapAction->setMenuRole(QAction::NoRole);
	connect(_loadMapAction, &QAction::triggered, this,
	  QOverload<>::of(&GUI::loadMap));
	_loadMapDirAction = new QAction(QIcon::fromTheme(OPEN_DIR_NAME,
	  QIcon(OPEN_DIR_ICON)), tr("Load map directory..."), this);
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
	  &MapView::showCursorCoordinates);
	QActionGroup *mapLayersGroup = new QActionGroup(this);
	connect(mapLayersGroup, &QActionGroup::triggered, this,
	  &GUI::selectMapLayers);
	_drawAllAction = new QAction(tr("All"), this);
	_drawAllAction->setMenuRole(QAction::NoRole);
	_drawAllAction->setCheckable(true);
	_drawAllAction->setActionGroup(mapLayersGroup);
	_drawRastersAction = new QAction(tr("Raster only"), this);
	_drawRastersAction->setMenuRole(QAction::NoRole);
	_drawRastersAction->setCheckable(true);
	_drawRastersAction->setActionGroup(mapLayersGroup);
	_drawVectorsAction = new QAction(tr("Vector only"), this);
	_drawVectorsAction->setMenuRole(QAction::NoRole);
	_drawVectorsAction->setCheckable(true);
	_drawVectorsAction->setActionGroup(mapLayersGroup);

	// Position
	_showPositionAction = new QAction(QIcon::fromTheme(SHOW_POS_NAME,
	  QIcon(SHOW_POS_ICON)), tr("Show position"), this);
	_showPositionAction->setMenuRole(QAction::NoRole);
	_showPositionAction->setCheckable(true);
	_showPositionAction->setEnabled(false);
	connect(_showPositionAction, &QAction::triggered, _mapView,
	  &MapView::showPosition);
	_followPositionAction = new QAction(tr("Follow position"), this);
	_followPositionAction->setMenuRole(QAction::NoRole);
	_followPositionAction->setCheckable(true);
	connect(_followPositionAction, &QAction::triggered, _mapView,
	  &MapView::followPosition);
	_showPositionCoordinatesAction = new QAction(tr("Show coordinates"),
	  this);
	_showPositionCoordinatesAction->setMenuRole(QAction::NoRole);
	_showPositionCoordinatesAction->setCheckable(true);
	connect(_showPositionCoordinatesAction, &QAction::triggered, _mapView,
	  &MapView::showPositionCoordinates);
	_showMotionInfoAction = new QAction(tr("Show motion info"), this);
	_showMotionInfoAction->setMenuRole(QAction::NoRole);
	_showMotionInfoAction->setCheckable(true);
	connect(_showMotionInfoAction, &QAction::triggered, _mapView,
	  &MapView::showMotionInfo);

	// Data actions
	_showTracksAction = new QAction(tr("Show tracks"), this);
	_showTracksAction->setMenuRole(QAction::NoRole);
	_showTracksAction->setCheckable(true);
	_showTracksAction->setShortcut(SHOW_TRACKS_SHORTCUT);
	connect(_showTracksAction, &QAction::triggered, this, &GUI::showTracks);
	_showRoutesAction = new QAction(tr("Show routes"), this);
	_showRoutesAction->setMenuRole(QAction::NoRole);
	_showRoutesAction->setCheckable(true);
	_showRoutesAction->setShortcut(SHOW_ROUTES_SHORTCUT);
	connect(_showRoutesAction, &QAction::triggered, this, &GUI::showRoutes);
	_showWaypointsAction = new QAction(tr("Show waypoints"), this);
	_showWaypointsAction->setMenuRole(QAction::NoRole);
	_showWaypointsAction->setCheckable(true);
	_showWaypointsAction->setShortcut(SHOW_WAYPOINTS_SHORTCUT);
	connect(_showWaypointsAction, &QAction::triggered, this,
	  &GUI::showWaypoints);
	_showAreasAction = new QAction(tr("Show areas"), this);
	_showAreasAction->setMenuRole(QAction::NoRole);
	_showAreasAction->setCheckable(true);
	_showAreasAction->setShortcut(SHOW_AREAS_SHORTCUT);
	connect(_showAreasAction, &QAction::triggered, this, &GUI::showAreas);
	_showWaypointIconsAction = new QAction(tr("Waypoint icons"), this);
	_showWaypointIconsAction->setMenuRole(QAction::NoRole);
	_showWaypointIconsAction->setCheckable(true);
	connect(_showWaypointIconsAction, &QAction::triggered, _mapView,
	  &MapView::showWaypointIcons);
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
	_useStylesAction = new QAction(tr("Use styles"), this);
	_useStylesAction->setMenuRole(QAction::NoRole);
	_useStylesAction->setCheckable(true);
	connect(_useStylesAction, &QAction::triggered, _mapView,
	  &MapView::useStyles);

	// DEM actions
	_downloadDataDEMAction = new QAction(tr("Download data DEM"), this);
	_downloadDataDEMAction->setMenuRole(QAction::NoRole);
	_downloadDataDEMAction->setEnabled(false);
	_downloadDataDEMAction->setShortcut(DOWNLOAD_DEM_SHORTCUT);
	connect(_downloadDataDEMAction, &QAction::triggered, this,
	  &GUI::downloadDataDEM);
	_downloadMapDEMAction = new QAction(tr("Download map DEM"), this);
	_downloadMapDEMAction->setMenuRole(QAction::NoRole);
	_downloadMapDEMAction->setEnabled(false);
	connect(_downloadMapDEMAction, &QAction::triggered, this,
	  &GUI::downloadMapDEM);
	_showDEMTilesAction = new QAction(tr("Show local DEM tiles"), this);
	_showDEMTilesAction->setMenuRole(QAction::NoRole);
	connect(_showDEMTilesAction, &QAction::triggered, this, &GUI::showDEMTiles);
	_drawHillShadingAction = new QAction(tr("Show hillshading"), this);
	_drawHillShadingAction->setMenuRole(QAction::NoRole);
	_drawHillShadingAction->setCheckable(true);
	connect(_drawHillShadingAction, &QAction::triggered, _mapView,
	  &MapView::drawHillShading);

	// Graph actions
	_showGraphsAction = new QAction(QIcon::fromTheme(SHOW_GRAPHS_NAME,
	  QIcon(SHOW_GRAPHS_ICON)), tr("Show graphs"), this);
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
#ifdef Q_OS_ANDROID
	_showGraphTabsAction = new QAction(tr("Show tabs"), this);
	_showGraphTabsAction->setMenuRole(QAction::NoRole);
	_showGraphTabsAction->setCheckable(true);
	connect(_showGraphTabsAction, &QAction::triggered, this,
	  &GUI::showGraphTabs);
#endif // Q_OS_ANDROID

	// Settings actions
#ifndef Q_OS_ANDROID
	_showToolbarsAction = new QAction(tr("Show toolbars"), this);
	_showToolbarsAction->setMenuRole(QAction::NoRole);
	_showToolbarsAction->setCheckable(true);
	connect(_showToolbarsAction, &QAction::triggered, this, &GUI::showToolbars);
#endif // Q_OS_ANDROID
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
#ifndef Q_OS_ANDROID
	_fullscreenAction = new QAction(QIcon::fromTheme(FULLSCREEN_NAME,
	  QIcon(FULLSCREEN_ICON)), tr("Fullscreen mode"), this);
	_fullscreenAction->setMenuRole(QAction::NoRole);
	_fullscreenAction->setCheckable(true);
	_fullscreenAction->setShortcut(FULLSCREEN_SHORTCUT);
	connect(_fullscreenAction, &QAction::triggered, this, &GUI::showFullscreen);
	addAction(_fullscreenAction);
#endif // Q_OS_ANDROID
	_openOptionsAction = new QAction(tr("Options..."), this);
	_openOptionsAction->setMenuRole(QAction::PreferencesRole);
	connect(_openOptionsAction, &QAction::triggered, this, &GUI::openOptions);

	// Navigation actions
#ifndef Q_OS_ANDROID
	_nextAction = new QAction(QIcon::fromTheme(NEXT_FILE_NAME,
	  QIcon(NEXT_FILE_ICON)), tr("Next"), this);
	_nextAction->setActionGroup(_navigationActionGroup);
	_nextAction->setMenuRole(QAction::NoRole);
	connect(_nextAction, &QAction::triggered, this, &GUI::next);
	_prevAction = new QAction(QIcon::fromTheme(PREV_FILE_NAME,
	  QIcon(PREV_FILE_ICON)), tr("Previous"), this);
	_prevAction->setMenuRole(QAction::NoRole);
	_prevAction->setActionGroup(_navigationActionGroup);
	connect(_prevAction, &QAction::triggered, this, &GUI::prev);
	_lastAction = new QAction(QIcon::fromTheme(LAST_FILE_NAME,
	  QIcon(LAST_FILE_ICON)), tr("Last"), this);
	_lastAction->setMenuRole(QAction::NoRole);
	_lastAction->setActionGroup(_navigationActionGroup);
	connect(_lastAction, &QAction::triggered, this, &GUI::last);
	_firstAction = new QAction(QIcon::fromTheme(FIRST_FILE_NAME,
	  QIcon(FIRST_FILE_ICON)), tr("First"), this);
	_firstAction->setMenuRole(QAction::NoRole);
	_firstAction->setActionGroup(_navigationActionGroup);
	connect(_firstAction, &QAction::triggered, this, &GUI::first);
#endif // Q_OS_ANDROID
}

void GUI::createMapNodeMenu(const TreeNode<MapAction*> &node, QMenu *menu,
  QAction *action)
{
	for (int i = 0; i < node.childs().size(); i++) {
		QMenu *cm = new QMenu(node.childs().at(i).name(), menu);
		menu->insertMenu(action, cm);
		createMapNodeMenu(node.childs().at(i), cm);
	}

	for (int i = 0; i < node.items().size(); i++)
		menu->insertAction(action, node.items().at(i));
}

void GUI::createPOINodeMenu(const TreeNode<POIAction*> &node, QMenu *menu,
  QAction *action)
{
	for (int i = 0; i < node.childs().size(); i++) {
		QMenu *cm = new QMenu(node.childs().at(i).name(), menu);
		menu->insertMenu(action, cm);
		createPOINodeMenu(node.childs().at(i), cm);
	}

	for (int i = 0; i < node.items().size(); i++)
		menu->insertAction(action, node.items().at(i));
}

void GUI::createMenus()
{
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(_openFileAction);
#ifndef Q_OS_ANDROID
	_recentFilesMenu = fileMenu->addMenu(tr("Open recent"));
	_recentFilesMenu->setIcon(QIcon::fromTheme(OPEN_RECENT_NAME,
	  QIcon(OPEN_RECENT_ICON)));
	_recentFilesMenu->menuAction()->setMenuRole(QAction::NoRole);
	_recentFilesMenu->setEnabled(false);
	_recentFilesEnd = _recentFilesMenu->addSeparator();
	_recentFilesMenu->addAction(_clearRecentFilesAction);
#endif // Q_OS_ANDROID
	fileMenu->addAction(_openDirAction);
	fileMenu->addSeparator();
#ifndef Q_OS_ANDROID
	fileMenu->addAction(_printFileAction);
#endif // Q_OS_ANDROID
	fileMenu->addAction(_exportPDFFileAction);
	fileMenu->addAction(_exportPNGFileAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_statisticsAction);
	fileMenu->addSeparator();
	fileMenu->addAction(_reloadFileAction);
	fileMenu->addAction(_closeFileAction);
#if !defined(Q_OS_MAC) && !defined(Q_OS_ANDROID)
	fileMenu->addSeparator();
	fileMenu->addAction(_exitAction);
#endif // Q_OS_MAC + Q_OS_ANDROID

	_mapMenu = menuBar()->addMenu(tr("&Map"));
	_mapsEnd = _mapMenu->addSeparator();
	_mapMenu->addAction(_loadMapAction);
	_mapMenu->addAction(_loadMapDirAction);
	_mapMenu->addAction(_clearMapCacheAction);
	_mapMenu->addSeparator();
	QMenu *layersMenu = _mapMenu->addMenu(tr("Layers"));
	layersMenu->menuAction()->setMenuRole(QAction::NoRole);
	layersMenu->addAction(_drawAllAction);
	layersMenu->addAction(_drawRastersAction);
	layersMenu->addAction(_drawVectorsAction);
	_mapMenu->addAction(_showCoordinatesAction);
	_mapMenu->addSeparator();
	_mapMenu->addAction(_showMapAction);

	QMenu *graphMenu = menuBar()->addMenu(tr("&Graph"));
	graphMenu->addAction(_distanceGraphAction);
	graphMenu->addAction(_timeGraphAction);
	graphMenu->addSeparator();
	graphMenu->addAction(_showGraphGridAction);
	graphMenu->addAction(_showGraphSliderInfoAction);
#ifdef Q_OS_ANDROID
	graphMenu->addAction(_showGraphTabsAction);
#endif // Q_OS_ANDROID
	graphMenu->addSeparator();
	graphMenu->addAction(_showGraphsAction);

	QMenu *dataMenu = menuBar()->addMenu(tr("&Data"));
	dataMenu->addAction(_showWaypointIconsAction);
	dataMenu->addAction(_showWaypointLabelsAction);
	dataMenu->addAction(_showRouteWaypointsAction);
	dataMenu->addAction(_showTicksAction);
	QMenu *markerMenu = dataMenu->addMenu(tr("Position info"));
	markerMenu->menuAction()->setMenuRole(QAction::NoRole);
	markerMenu->addAction(_hideMarkersAction);
	markerMenu->addAction(_showMarkersAction);
	markerMenu->addAction(_showMarkerDateAction);
	markerMenu->addAction(_showMarkerCoordinatesAction);
	dataMenu->addSeparator();
	dataMenu->addAction(_useStylesAction);
	dataMenu->addSeparator();
	dataMenu->addAction(_showTracksAction);
	dataMenu->addAction(_showRoutesAction);
	dataMenu->addAction(_showAreasAction);
	dataMenu->addAction(_showWaypointsAction);

	_poiMenu = menuBar()->addMenu(tr("&POI"));
	_poisEnd = _poiMenu->addSeparator();
	_poiMenu->addAction(_openPOIAction);
	_poiMenu->addAction(_selectAllPOIAction);
	_poiMenu->addAction(_unselectAllPOIAction);
	_poiMenu->addSeparator();
	_poiMenu->addAction(_showPOIIconsAction);
	_poiMenu->addAction(_showPOILabelsAction);
	_poiMenu->addAction(_overlapPOIAction);
	_poiMenu->addSeparator();
	_poiMenu->addAction(_showPOIAction);

	QMenu *demMenu = menuBar()->addMenu(tr("DEM"));
	demMenu->addAction(_showDEMTilesAction);
	demMenu->addSeparator();
	demMenu->addAction(_downloadDataDEMAction);
	demMenu->addAction(_downloadMapDEMAction);
	demMenu->addSeparator();
	demMenu->addAction(_drawHillShadingAction);

	QMenu *positionMenu = menuBar()->addMenu(tr("Position"));
	positionMenu->addAction(_showPositionCoordinatesAction);
	positionMenu->addAction(_showMotionInfoAction);
	positionMenu->addAction(_followPositionAction);
	positionMenu->addSeparator();
	positionMenu->addAction(_showPositionAction);

	QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
	QMenu *timeMenu = settingsMenu->addMenu(tr("Time"));
	timeMenu->menuAction()->setMenuRole(QAction::NoRole);
	timeMenu->addAction(_totalTimeAction);
	timeMenu->addAction(_movingTimeAction);
	QMenu *unitsMenu = settingsMenu->addMenu(tr("Units"));
	unitsMenu->menuAction()->setMenuRole(QAction::NoRole);
	unitsMenu->addAction(_metricUnitsAction);
	unitsMenu->addAction(_imperialUnitsAction);
	unitsMenu->addAction(_nauticalUnitsAction);
	QMenu *coordinatesMenu = settingsMenu->addMenu(tr("Coordinates format"));
	coordinatesMenu->menuAction()->setMenuRole(QAction::NoRole);
	coordinatesMenu->addAction(_decimalDegreesAction);
	coordinatesMenu->addAction(_degreesMinutesAction);
	coordinatesMenu->addAction(_dmsAction);
	settingsMenu->addSeparator();
#ifndef Q_OS_ANDROID
	settingsMenu->addAction(_showToolbarsAction);
	settingsMenu->addAction(_fullscreenAction);
	settingsMenu->addSeparator();
#endif // Q_OS_ANDROID
	settingsMenu->addAction(_openOptionsAction);

	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(_pathsAction);
#ifndef Q_OS_ANDROID
	helpMenu->addAction(_keysAction);
#endif // Q_OS_ANDROID
	helpMenu->addSeparator();
	helpMenu->addAction(_aboutAction);
}

#ifdef Q_OS_ANDROID
void GUI::createNavigation()
{
	_navigation = new NavigationWidget(_mapView);

	connect(_navigation, &NavigationWidget::next, this, &GUI::next);
	connect(_navigation, &NavigationWidget::prev, this, &GUI::prev);
}
#else // Q_OS_ANDROID
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
#ifndef Q_OS_MAC
	_fileToolBar->addAction(_printFileAction);
#endif // Q_OS_MAC

	_showToolBar = addToolBar(tr("Show"));
	_showToolBar->setObjectName("Show");
	_showToolBar->setIconSize(iconSize);
	_showToolBar->addAction(_showPOIAction);
	_showToolBar->addAction(_showMapAction);
	_showToolBar->addAction(_showGraphsAction);
	_showToolBar->addAction(_showPositionAction);

	_navigationToolBar = addToolBar(tr("Navigation"));
	_navigationToolBar->setObjectName("Navigation");
	_navigationToolBar->setIconSize(iconSize);
	_navigationToolBar->addAction(_firstAction);
	_navigationToolBar->addAction(_prevAction);
	_navigationToolBar->addAction(_nextAction);
	_navigationToolBar->addAction(_lastAction);
}
#endif // Q_OS_ANDROID

void GUI::createMapView()
{
	_map = new EmptyMap(this);
	_mapView = new MapView(_map, _poi, this);
	_mapView->setSizePolicy(QSizePolicy(QSizePolicy::Ignored,
	  QSizePolicy::Expanding));
#ifdef Q_OS_ANDROID
	_mapView->setMinimumHeight(100);
#else // Q_OS_ANDROID
	_mapView->setMinimumHeight(200);
#endif // Q_OS_ANDROID
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
#ifdef Q_OS_ANDROID
	msgBox.setText("<h2>" + QString(APP_NAME) + "</h2><p>" + tr("Version %1")
	  .arg(QString(APP_VERSION) + " (" + QSysInfo::buildCpuArchitecture()
	  + ", Qt " + QT_VERSION_STR + ")") + "</p><p>"
	  + tr("GPXSee is distributed under the terms of the GNU General Public "
	  "License version 3. For more info about GPXSee visit the project "
	  "homepage at %1.").arg("<a href=\"" + homepage.toString() + "\">"
	  + homepage.toString(QUrl::RemoveScheme).mid(2) + "</a>") + "</p>");
#else // Q_OS_ANDROID
	msgBox.setText("<h2>" + QString(APP_NAME) + "</h2><p>" + tr("Version %1")
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
#endif // Q_OS_ANDROID

	msgBox.exec();
}

#ifndef Q_OS_ANDROID
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
#endif // Q_OS_ANDROID

void GUI::paths()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle(tr("Paths"));
#ifdef Q_OS_ANDROID
	msgBox.setText(
	  + "<small><b>" + tr("Map directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::mapDir(true))	+ "<br><br><b>"
	  + tr("POI directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::poiDir(true)) + "<br><br><b>"
	  + tr("CRS directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::crsDir(true)) + "<br><br><b>"
	  + tr("DEM directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::demDir(true)) + "<br><br><b>"
	  + tr("Styles directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::styleDir(true)) + "<br><br><b>"
	  + tr("Symbols directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::symbolsDir(true)) + "<br><br><b>"
	  + tr("Tile cache directory:") + "</b><br>"
	  + QDir::cleanPath(ProgramPaths::tilesDir()) + "</small>");
#else // Q_OS_ANDROID
	msgBox.setText("<h3>" + tr("Paths") + "</h3>");
	msgBox.setInformativeText(
	  "<style>td {white-space: pre; padding-right: 1em;}</style><table><tr><td>"
	  + tr("Map directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::mapDir(true)) + "</code></td></tr><tr><td>"
	  + tr("POI directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::poiDir(true)) + "</code></td></tr><tr><td>"
	  + tr("CRS directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::crsDir(true)) + "</code></td></tr><tr><td>"
	  + tr("DEM directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::demDir(true)) + "</code></td></tr><tr><td>"
	  + tr("Styles directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::styleDir(true)) + "</code></td></tr><tr><td>"
	  + tr("Symbols directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::symbolsDir(true)) + "</code></td></tr><tr><td>"
	  + tr("Tile cache directory:") + "</td><td><code>"
	  + QDir::cleanPath(ProgramPaths::tilesDir()) + "</code></td></tr></table>"
	);
#endif // Q_OS_ANDROID

	msgBox.exec();
}

void GUI::openFile()
{
#ifdef Q_OS_ANDROID
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open file"),
	  _dataDir));
#else // Q_OS_ANDROID
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open file"),
	  _dataDir, Data::formats()));
#endif // Q_OS_ANDROID
	int showError = (files.size() > 1) ? 2 : 1;

	for (int i = 0; i < files.size(); i++)
		openFile(files.at(i), true, showError);
	if (!files.isEmpty())
		_dataDir = QFileInfo(files.last()).path();
}

#ifndef Q_OS_ANDROID
void GUI::openDir(const QString &path, int &showError)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);

		if (fi.isDir())
			openDir(fi.absoluteFilePath(), showError);
		else
			openFile(fi.absoluteFilePath(), true, showError);
	}
}
#endif // Q_OS_ANDROID

void GUI::openDir()
{
	QString dir(QFileDialog::getExistingDirectory(this, tr("Open directory"),
	  _dataDir));

	if (!dir.isEmpty()) {
#ifdef Q_OS_ANDROID
		int showError = 1;
		_browser->setCurrentDir(dir);
		openFile(_browser->current(), true, showError);
#else // Q_OS_ANDROID
		int showError = 2;
		openDir(dir, showError);
		_dataDir = dir;
#endif // Q_OS_ANDROID
	}
}

bool GUI::openFile(const QString &fileName, bool tryUnknown, int &showError)
{
	QString path;

	QUrl url(fileName);
	if (url.scheme() == "geo") {
		if (loadURL(url, showError)) {
			_fileActionGroup->setEnabled(true);
			_reloadFileAction->setEnabled(false);
			return true;
		} else if (showError)
			return false;
	} else if (url.isLocalFile())
		path = url.toLocalFile();
	else
		path = fileName;

	QFileInfo fi(path);
	QString canonicalPath(fi.canonicalFilePath());

	if (_files.contains(canonicalPath))
		return true;

	if (!loadFile(path, tryUnknown, showError))
		return false;

	_files.append(canonicalPath);
#ifndef Q_OS_ANDROID
	_browser->setCurrent(path);
#endif // Q_OS_ANDROID
	_fileActionGroup->setEnabled(true);
	// Explicitly enable the reload action as it may be disabled by loadMapDir()
	_reloadFileAction->setEnabled(true);
	_navigationActionGroup->setEnabled(true);

	updateNavigationActions();
	updateStatusBarInfo();
	updateWindowTitle();
	if (_files.count() > 1)
		_mapView->showExtendedInfo(true);
#ifndef Q_OS_ANDROID
	updateRecentFiles(canonicalPath);
#endif // Q_OS_ANDROID

	return true;
}

bool GUI::loadURL(const QUrl &url, int &showError)
{
	Data data(url);

	if (data.isValid()) {
		loadData(data);
		return true;
	} else {
		if (showError) {
			QString error = tr("Error loading geo URI:") + "\n" + url.toString()
			  + ": " + data.errorString();

			if (showError > 1) {
				QMessageBox message(QMessageBox::Critical, APP_NAME, error,
				  QMessageBox::Ok, this);
				QCheckBox checkBox(tr("Don't show again"));
				message.setCheckBox(&checkBox);
				message.exec();
				if (checkBox.isChecked())
					showError = 0;
			} else
				QMessageBox::critical(this, APP_NAME, error);
		} else
			qWarning("%s: %s", qUtf8Printable(url.toString()),
			  qUtf8Printable(data.errorString()));

		return false;
	}
}

bool GUI::loadFile(const QString &fileName, bool tryUnknown, int &showError)
{
	Data data(fileName, tryUnknown);

	if (data.isValid()) {
		loadData(data);
		return true;
	} else {
		updateNavigationActions();
		updateStatusBarInfo();
		updateWindowTitle();
		updateGraphTabs();
		updateDataDEMDownloadAction();
		if (_files.isEmpty())
			_fileActionGroup->setEnabled(false);

		if (showError) {
			QString error = tr("Error loading data file:") + "\n"
			  + Util::displayName(fileName) + ": " + data.errorString();
			if (data.errorLine())
				error.append("\n" + tr("Line: %1").arg(data.errorLine()));

			if (showError > 1) {
				QMessageBox message(QMessageBox::Critical, APP_NAME, error,
				  QMessageBox::Ok, this);
				QCheckBox checkBox(tr("Don't show again"));
				message.setCheckBox(&checkBox);
				message.exec();
				if (checkBox.isChecked())
					showError = 0;
			} else
				QMessageBox::critical(this, APP_NAME, error);
		}

		return false;
	}
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
		graphs.append(_tabs.at(i)->loadData(data, _map));
	/* Refreshing the splitter is necessary to update the map viewport and
	   properly fit the data! */
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

	updateDataDEMDownloadAction();
}

void GUI::openPOIFile()
{
#ifdef Q_OS_ANDROID
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open POI file"),
	  _poiDir));
#else // Q_OS_ANDROID
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open POI file"),
	  _poiDir, Data::formats()));
#endif // Q_OS_ANDROID

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
		QString error = tr("Error loading POI file:") + "\n"
		  + Util::displayName(fileName) + ": " + _poi->errorString();
		if (_poi->errorLine())
			error.append("\n" + tr("Line: %1").arg(_poi->errorLine()));
		QMessageBox::critical(this, APP_NAME, error);

		return false;
	}
}

void GUI::openOptions()
{
	Options options(_options);
	OptionsDialog dialog(options, _units, this);

	if (dialog.exec() != QDialog::Accepted)
		return;

	updateOptions(options);
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

static void header(QString &text)
{
#ifdef Q_OS_ANDROID
	Q_UNUSED(text);
#else // Q_OS_ANDROID
#ifdef Q_OS_WIN32
	text = "<style>td {white-space: pre; padding-right: 4em;}"
	  "th {text-align: left; padding-top: 0.5em;}</style><table>";
#else // Q_OS_WIN32
	text = "<style>td {white-space: pre; padding-right: 2em;}"
	  "th {text-align: left; padding-top: 0.5em;}</style><table>";
#endif // Q_OS_WIN32
#endif // Q_OS_ANDROID
}

static void footer(QString &text)
{
#ifdef Q_OS_ANDROID
	Q_UNUSED(text);
#else // Q_OS_ANDROID
	text.append("</table>");
#endif // Q_OS_ANDROID
}

static void appendRow(const QString &key, const QString &value, QString &text)
{
#ifdef Q_OS_ANDROID
	text.append("<b>" + key + ":</b> " + value + "<br>");
#else // Q_OS_ANDROID
	text.append("<tr><td>" + key + ":</td><td>" + value + "</td></tr>");
#endif // Q_OS_ANDROID
}

static void appendGraphInfo(const GraphTab *tab, QString &text)
{
#ifdef Q_OS_ANDROID
	text.append("<br><i>" + tab->label() + "</i><br>");
	for (int j = 0; j < tab->info().size(); j++) {
		const KV<QString, QString> &kv = tab->info().at(j);
		if (j)
			text.append(" | ");
		text.append("<b>" + kv.key() + ":</b>&nbsp;" + kv.value());
	}
	text.append("<br>");
#else // Q_OS_ANDROID
	text.append("<tr><th colspan=\"2\">" + tab->label() + "</th></tr>");
	for (int j = 0; j < tab->info().size(); j++) {
		const KV<QString, QString> &kv = tab->info().at(j);
		text.append("<tr><td>" + kv.key() + ":</td><td>" + kv.value()
		  + "</td></tr>");
	}
#endif // Q_OS_ANDROID
}

void GUI::statistics()
{
	QLocale l(QLocale::system());
	QMessageBox msgBox(this);
	QString text;

	header(text);

	if (_showTracksAction->isChecked() && _trackCount > 1)
		appendRow(tr("Tracks"), l.toString(_trackCount), text);
	if (_showRoutesAction->isChecked() && _routeCount > 1)
		appendRow(tr("Routes"), l.toString(_routeCount), text);
	if (_showWaypointsAction->isChecked() && _waypointCount > 1)
		appendRow(tr("Waypoints"), l.toString(_waypointCount), text);
	if (_showAreasAction->isChecked() && _areaCount > 1)
		appendRow(tr("Areas"), l.toString(_areaCount), text);

	if (_dateRange.first.isValid()) {
		if (_dateRange.first == _dateRange.second)
			appendRow(tr("Date"), l.toString(_dateRange.first.date()), text);
		else
			appendRow(tr("Date"), QString("%1 - %2").arg(
			  l.toString(_dateRange.first.date(), QLocale::ShortFormat),
			  l.toString(_dateRange.second.date(), QLocale::ShortFormat)), text);
	}

	if (distance() > 0)
		appendRow(tr("Distance"), Format::distance(distance(), units()), text);
	if (time() > 0) {
		appendRow(tr("Time"), Format::timeSpan(time()), text);
		appendRow(tr("Moving time"), Format::timeSpan(movingTime()), text);
	}

	for (int i = 0; i < _tabs.count(); i++) {
		const GraphTab *tab = _tabs.at(i);
		if (!tab->isEmpty())
			appendGraphInfo(tab, text);
	}

	footer(text);

#ifdef Q_OS_ANDROID
	msgBox.setWindowTitle(tr("Statistics"));
	msgBox.setText(text);
#else // Q_OS_ANDROID
	msgBox.setWindowTitle(tr("Statistics"));
	msgBox.setText("<h3>" + tr("Statistics") + "</h3>");
	msgBox.setInformativeText(text);
#endif // Q_OS_ANDROID

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
		if (_dateRange.first == _dateRange.second)
			info.insert(tr("Date"), l.toString(_dateRange.first.date()));
		else {
			info.insert(tr("Date"), QString("%1 - %2")
			  .arg(l.toString(_dateRange.first.date(), QLocale::ShortFormat),
			  l.toString(_dateRange.second.date(), QLocale::ShortFormat)));
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

	MapView::Flags flags;
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

	int showError = 2;
	for (int i = 0; i < _files.size(); i++) {
		if (!loadFile(_files.at(i), true, showError)) {
			_files.removeAt(i);
			i--;
		}
	}

	updateStatusBarInfo();
	updateWindowTitle();
	if (_files.isEmpty())
		_fileActionGroup->setEnabled(false);
#ifndef Q_OS_ANDROID
	else
		_browser->setCurrent(_files.last());
#endif // Q_OS_ANDROID
	updateDataDEMDownloadAction();
	_mapView->showExtendedInfo(_files.size() > 1);
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
	_mapView->showExtendedInfo(false);

	_files.clear();
}

void GUI::closeAll()
{
	closeFiles();

	_fileActionGroup->setEnabled(false);
	updateStatusBarInfo();
	updateWindowTitle();
	updateGraphTabs();
	updateDataDEMDownloadAction();

#ifdef Q_OS_ANDROID
	_browser->setCurrentDir(QString());
#endif // Q_OS_ANDROID
}

void GUI::showGraphs(bool show)
{
	_graphTabWidget->setHidden(!show);
}

#ifdef Q_OS_ANDROID
void GUI::showGraphTabs(bool show)
{
	_graphTabWidget->tabBar()->setVisible(show);
}
#else // Q_OS_ANDROID
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
#endif // Q_OS_ANDROID

void GUI::showTracks(bool show)
{
	_mapView->showTracks(show);

	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showTracks(show);

	updateStatusBarInfo();
	updateGraphTabs();
	updateDataDEMDownloadAction();
}

void GUI::showRoutes(bool show)
{
	_mapView->showRoutes(show);

	for (int i = 0; i < _tabs.size(); i++)
		_tabs.at(i)->showRoutes(show);

	updateStatusBarInfo();
	updateGraphTabs();
	updateDataDEMDownloadAction();
}

void GUI::showWaypoints(bool show)
{
	_mapView->showWaypoints(show);
	updateDataDEMDownloadAction();
}

void GUI::showAreas(bool show)
{
	_mapView->showAreas(show);
	updateDataDEMDownloadAction();
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

void GUI::selectMapLayers(QAction *action)
{
	if (action == _drawVectorsAction)
		_mapView->selectLayers(MapView::Layer::Vector);
	else if (action == _drawRastersAction)
		_mapView->selectLayers(MapView::Layer::Raster);
	else
		_mapView->selectLayers(MapView::Layer::Raster | MapView::Layer::Vector);
}

void GUI::loadMap()
{
#ifdef Q_OS_ANDROID
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open map file"),
	  _mapDir));
#else // Q_OS_ANDROID
	QStringList files(QFileDialog::getOpenFileNames(this, tr("Open map file"),
	  _mapDir, MapList::formats()));
#endif // Q_OS_ANDROID
	MapAction *a, *lastReady = 0;
	int showError = (files.size() > 1) ? 2 : 1;

	for (int i = 0; i < files.size(); i++) {
		if (loadMap(files.at(i), a, showError) && a)
			lastReady = a;
	}
	if (!files.isEmpty())
		_mapDir = QFileInfo(files.last()).path();
	if (lastReady)
		lastReady->trigger();
}

void GUI::reloadMap()
{
	_mapView->setMap(_map);
	updateMapDEMDownloadAction();
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
  const QList<QAction*> &existingActions, int &showError)
{
	bool valid = false;

	action = 0;

	for (int i = 0; i < node.childs().size(); i++)
		valid = loadMapNode(node.childs().at(i), action, existingActions,
		  showError);

	for (int i = 0; i < node.items().size(); i++) {
		Map *map = node.items().at(i);
		MapAction *a;

		if (!(a = findMapAction(existingActions, map))) {
			if (!map->isValid()) {
				if (showError) {
					QString error = tr("Error loading map:") + "\n"
					  + Util::displayName(map->path()) + ": " + map->errorString();

					if (showError > 1) {
						QMessageBox message(QMessageBox::Critical, APP_NAME,
						  error, QMessageBox::Ok, this);
						QCheckBox checkBox(tr("Don't show again"));
						message.setCheckBox(&checkBox);
						message.exec();
						if (checkBox.isChecked())
							showError = 0;
					} else
						QMessageBox::critical(this, APP_NAME, error);
				}

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

bool GUI::loadMap(const QString &fileName, MapAction *&action, int &showError)
{
	QString path;
	QUrl url(fileName);

	path = url.isLocalFile() ? url.toLocalFile() : fileName;


	TreeNode<Map*> maps(MapList::loadMaps(path, _mapView->inputProjection()));
	QList<QAction*> existingActions(_mapsActionGroup->actions());

	return loadMapNode(maps, action, existingActions, showError);
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
		QString error = tr("Error loading map:") + "\n"
		  + Util::displayName(map->path()) + ": " + map->errorString();
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
		QString error = tr("Error loading map:") + "\n"
		  + Util::displayName(map->path()) + ": " + map->errorString();
		QMessageBox::critical(this, APP_NAME, error);
		action->deleteLater();
	}
}

void GUI::loadMapDirNode(const TreeNode<Map *> &node, QList<MapAction*> &actions,
  QMenu *menu, const QList<QAction*> &existingActions, int &showError)
{
	for (int i = 0; i < node.childs().size(); i++) {
		QMenu *cm = new QMenu(node.childs().at(i).name(), menu);
		menu->addMenu(cm);
		loadMapDirNode(node.childs().at(i), actions, cm, existingActions,
		  showError);
	}

	for (int i = 0; i < node.items().size(); i++) {
		Map *map = node.items().at(i);
		MapAction *a;

		if (!(a = findMapAction(existingActions, map))) {
			if (!map->isValid()) {
				if (showError) {
					QString error = tr("Error loading map:") + "\n"
					  + Util::displayName(map->path()) + ": " + map->errorString();

					if (showError > 1) {
						QMessageBox message(QMessageBox::Critical, APP_NAME,
						  error, QMessageBox::Ok, this);
						QCheckBox checkBox(tr("Don't show again"));
						message.setCheckBox(&checkBox);
						message.exec();
						if (checkBox.isChecked())
							showError = 0;
					} else
						QMessageBox::critical(this, APP_NAME, error);
				}

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
	TreeNode<Map*> maps(MapList::loadMaps(dir, _mapView->inputProjection()));
	QList<QAction*> existingActions(_mapsActionGroup->actions());
	QList<MapAction*> actions;
	QMenu *menu = new QMenu(maps.name());
	int showError = (maps.items().size() > 1 || !maps.childs().isEmpty())
	  ? 2 : 1;

	loadMapDirNode(maps, actions, menu, existingActions, showError);

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
	  tr("Clear \"%1\" tile cache?").arg(_map->name())) == QMessageBox::Yes)
		_mapView->clearMapCache();
}

void GUI::downloadDataDEM()
{
	downloadDEM(_mapView->boundingRect());
}

void GUI::downloadMapDEM()
{
	downloadDEM(_map->llBounds());
}

void GUI::downloadDEM(const RectC &rect)
{
	int cnt = _dem->numTiles(rect);

	if (cnt > DEM_DOWNLOAD_LIMIT)
		QMessageBox::information(this, APP_NAME,
		  tr("DEM tiles download limit exceeded. If you really need data for "
		  "such a huge area, download the files manually."));
	else if (cnt < DEM_DOWNLOAD_WARNING || QMessageBox::question(this, APP_NAME,
	  tr("Download %n DEM tiles?", "", cnt)) == QMessageBox::Yes) {
		_demRects.append(rect);
		if (!_dem->loadTiles(rect) && _demRects.size() == 1)
			demLoaded();
	}
}


void GUI::demLoaded()
{
	for (int i = 0; i < _demRects.size(); i++) {
		if (!_dem->checkTiles(_demRects.at(i))) {
			QMessageBox::warning(this, APP_NAME,
			  tr("Could not download all required DEM files."));
			break;
		}
	}

	_demRects.clear();
	DEM::clearCache();

	reloadFiles();
	reloadMap();
}

void GUI::showDEMTiles()
{
	QList<Area> tiles(DEM::tiles());

	if (tiles.isEmpty()) {
		QMessageBox::information(this, APP_NAME, tr("No local DEM tiles found."));
	} else {
		_mapView->loadDEMs(tiles);

		_areaCount += tiles.size();

		_fileActionGroup->setEnabled(true);
		_reloadFileAction->setEnabled(false);
	}
}

void GUI::updateStatusBarInfo()
{
	if (_files.count() == 0)
		_fileNameLabel->setText(tr("No files loaded"));
	else if (_files.count() == 1)
		_fileNameLabel->setText(Util::displayName(_files.at(0)));
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

#ifdef Q_OS_ANDROID
	statusBar()->setVisible(!_files.isEmpty());
#endif // Q_OS_ANDROID
}

void GUI::updateWindowTitle()
{
	if (_files.count() == 1)
		setWindowTitle(QFileInfo(_files.at(0)).fileName() + " - " + APP_NAME);
	else
		setWindowTitle(APP_NAME);
}

#ifndef Q_OS_ANDROID
void GUI::updateRecentFiles(const QString &fileName)
{
	QAction *a = 0;

	QList<QAction *> actions(_recentFilesActionGroup->actions());
	for (int i = 0; i < actions.size(); i++) {
		if (actions.at(i)->data().toString() == fileName) {
			a = actions.at(i);
			break;
		}
	}

	if (a)
		delete a;
	else if (actions.size() == MAX_RECENT_FILES)
		delete actions.first();

	actions = _recentFilesActionGroup->actions();
	QAction *before = actions.size() ? actions.last() : _recentFilesEnd;
	a = new QAction(fileName, _recentFilesActionGroup);
	a->setData(fileName);
	_recentFilesMenu->insertAction(before, a);
	_recentFilesMenu->setEnabled(true);
}

void GUI::clearRecentFiles()
{
	QList<QAction *> actions(_recentFilesActionGroup->actions());

	for (int i = 0; i < actions.size(); i++)
		delete actions.at(i);

	_recentFilesMenu->setEnabled(false);
}

void GUI::recentFileSelected(QAction *action)
{
	int showError = 1;
	openFile(action->data().toString(), true, showError);
}
#endif // Q_OS_ANDROID

void GUI::mapChanged(QAction *action)
{
	_map = action->data().value<Map*>();
	_mapView->setMap(_map);
	updateMapDEMDownloadAction();
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
#ifdef Q_OS_ANDROID
	_navigation->enableNext(!_browser->isLast()
	  && !_browser->current().isNull());
	_navigation->enablePrev(!_browser->isFirst()
	  && !_browser->current().isNull());
#else // Q_OS_ANDROID
	_lastAction->setEnabled(!_browser->isLast());
	_nextAction->setEnabled(!_browser->isLast());
	_firstAction->setEnabled(!_browser->isFirst());
	_prevAction->setEnabled(!_browser->isFirst());
#endif // Q_OS_ANDROID
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

void GUI::updateDataDEMDownloadAction()
{
	_downloadDataDEMAction->setEnabled(!_dem->url().isEmpty()
	  && !_dem->checkTiles(_mapView->boundingRect()));
}

void GUI::updateMapDEMDownloadAction()
{
	_downloadMapDEMAction->setEnabled(!_dem->url().isEmpty()
	  && !_dem->checkTiles(_map->llBounds()));
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
	int showError = 1;
	QString file = _browser->next();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file, true, showError);
}

void GUI::prev()
{
	int showError = 1;
	QString file = _browser->prev();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file, true, showError);
}

void GUI::last()
{
	int showError = 1;
	QString file = _browser->last();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file, true, showError);
}

void GUI::first()
{
	int showError = 1;
	QString file = _browser->first();
	if (file.isNull())
		return;

	closeFiles();
	openFile(file, true, showError);
}

#ifndef Q_OS_ANDROID
void GUI::keyPressEvent(QKeyEvent *event)
{
	QString file;
	int showError = 1;

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
		openFile(file, true, showError);
		return;
	}

	QMainWindow::keyPressEvent(event);
}
#endif // Q_OS_ANDROID

void GUI::closeEvent(QCloseEvent *event)
{
	writeSettings();
	QMainWindow::closeEvent(event);
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
	int silent = 0;
	int showError = (urls.size() > 1) ? 2 : 1;

	for (int i = 0; i < urls.size(); i++) {
		QString file(urls.at(i).toLocalFile());

		if (!openFile(file, false, silent)) {
			MapAction *a;
			if (!loadMap(file, a, silent))
				openFile(file, true, showError);
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

QGeoPositionInfoSource *GUI::positionSource(const Options &options)
{
	QGeoPositionInfoSource *source;

	source = QGeoPositionInfoSource::createSource(options.plugin,
	  options.pluginParams.value(options.plugin), this);
	if (source)
		source->setPreferredPositioningMethods(
		  QGeoPositionInfoSource::SatellitePositioningMethods);

	return source;
}

void GUI::writeSettings()
{
#define WRITE(name, value) \
	Settings::name.write(settings, value);

	QSettings settings(qApp->applicationName(), qApp->applicationName());
	settings.clear();

	/* Window */
#ifndef Q_OS_ANDROID
	settings.beginGroup(SETTINGS_WINDOW);
	if (!_windowStates.isEmpty() && !_windowGeometries.isEmpty()) {
		WRITE(windowState, _windowStates.first());
		WRITE(windowGeometry, _windowGeometries.first());
	} else {
		WRITE(windowState, saveState());
		WRITE(windowGeometry, saveGeometry());
	}
	settings.endGroup();
#endif // Q_OS_ANDROID

	/* Settings */
	settings.beginGroup(SETTINGS_SETTINGS);
	WRITE(timeType, _movingTimeAction->isChecked() ? Moving : Total);
	WRITE(units, _imperialUnitsAction->isChecked()
	  ? Imperial : _nauticalUnitsAction->isChecked()
	  ? Nautical : Metric);
	WRITE(coordinatesFormat, _dmsAction->isChecked()
	  ? DMS : _degreesMinutesAction->isChecked()
	  ? DegreesMinutes : DecimalDegrees);
#ifndef Q_OS_ANDROID
	WRITE(showToolbars, _showToolbarsAction->isChecked());
#endif // Q_OS_ANDROID
	settings.endGroup();

	/* File */
#ifndef Q_OS_ANDROID
	QList<QAction*> recentActions(_recentFilesActionGroup->actions());
	QStringList recent;
	for (int i = 0; i < recentActions.size(); i++)
		recent.append(recentActions.at(i)->data().toString());

	settings.beginGroup(SETTINGS_FILE);
	WRITE(recentDataFiles, recent);
	settings.endGroup();
#endif // Q_OS_ANDROID

	/* Map */
	MapView::Layers ml;
	if (_drawRastersAction->isChecked())
		ml = MapView::Layer::Raster;
	else if (_drawVectorsAction->isChecked())
		ml = MapView::Layer::Vector;
	else
		ml = MapView::Layer::Raster | MapView::Layer::Vector;

	settings.beginGroup(SETTINGS_MAP);
	WRITE(activeMap, _map->name());
	WRITE(showMap, _showMapAction->isChecked());
	WRITE(cursorCoordinates, _showCoordinatesAction->isChecked());
	WRITE(layers, (int)ml);
	settings.endGroup();

	/* Graph */
	settings.beginGroup(SETTINGS_GRAPH);
	WRITE(showGraphs, _showGraphsAction->isChecked());
	WRITE(graphType, _timeGraphAction->isChecked() ? Time : Distance);
	WRITE(showGrid, _showGraphGridAction->isChecked());
	WRITE(sliderInfo, _showGraphSliderInfoAction->isChecked());
#ifdef Q_OS_ANDROID
	WRITE(showGraphTabs, _showGraphTabsAction->isChecked());
#endif // Q_OS_ANDROID
	settings.endGroup();

	/* POI */
	QList<QAction*> disabledActions(_poisActionGroup->actions());
	QStringList disabled;
	for (int i = 0; i < disabledActions.size(); i++)
		if (!disabledActions.at(i)->isChecked())
			disabled.append(disabledActions.at(i)->data().toString());

	settings.beginGroup(SETTINGS_POI);
	WRITE(showPoi, _showPOIAction->isChecked());
	WRITE(poiOverlap, _overlapPOIAction->isChecked());
	WRITE(poiLabels, _showPOILabelsAction->isChecked());
	WRITE(poiIcons, _showPOIIconsAction->isChecked());
	WRITE(disabledPoiFiles, disabled);
	settings.endGroup();

	/* Data */
	MarkerInfoItem::Type mi;
	if (_showMarkerDateAction->isChecked())
		mi = MarkerInfoItem::Date;
	else if (_showMarkerCoordinatesAction->isChecked())
		mi = MarkerInfoItem::Position;
	else
		mi = MarkerInfoItem::None;

	settings.beginGroup(SETTINGS_DATA);
	WRITE(tracks, _showTracksAction->isChecked());
	WRITE(routes, _showRoutesAction->isChecked());
	WRITE(waypoints, _showWaypointsAction->isChecked());
	WRITE(areas, _showAreasAction->isChecked());
	WRITE(waypointIcons, _showWaypointIconsAction->isChecked());
	WRITE(waypointLabels, _showWaypointLabelsAction->isChecked());
	WRITE(routeWaypoints, _showRouteWaypointsAction->isChecked());
	WRITE(pathTicks, _showTicksAction->isChecked());
	WRITE(positionMarkers, _showMarkersAction->isChecked()
	  || _showMarkerDateAction->isChecked()
	  || _showMarkerCoordinatesAction->isChecked());
	WRITE(markerInfo, mi);
	WRITE(useStyles, _useStylesAction->isChecked());
	settings.endGroup();

	/* DEM */
	settings.beginGroup(SETTINGS_DEM);
	WRITE(drawHillShading, _drawHillShadingAction->isChecked());
	settings.endGroup();

	/* Position */
	settings.beginGroup(SETTINGS_POSITION);
	WRITE(showPosition, _showPositionAction->isChecked());
	WRITE(followPosition, _followPositionAction->isChecked());
	WRITE(positionCoordinates, _showPositionCoordinatesAction->isChecked());
	WRITE(motionInfo, _showMotionInfoAction->isChecked());
	settings.endGroup();

	/* PDF export */
	settings.beginGroup(SETTINGS_PDF_EXPORT);
	WRITE(pdfOrientation, _pdfExport.orientation);
	WRITE(pdfResolution, _pdfExport.resolution);
	WRITE(pdfSize, _pdfExport.paperSize);
	WRITE(pdfMarginLeft, _pdfExport.margins.left());
	WRITE(pdfMarginTop, _pdfExport.margins.top());
	WRITE(pdfMarginRight, _pdfExport.margins.right());
	WRITE(pdfMarginBottom, _pdfExport.margins.bottom());
	WRITE(pdfFileName, _pdfExport.fileName);
	settings.endGroup();

	/* PNG export */
	settings.beginGroup(SETTINGS_PNG_EXPORT);
	WRITE(pngWidth, _pngExport.size.width());
	WRITE(pngHeight, _pngExport.size.height());
	WRITE(pngMarginLeft, _pngExport.margins.left());
	WRITE(pngMarginTop, _pngExport.margins.top());
	WRITE(pngMarginRight, _pngExport.margins.right());
	WRITE(pngMarginBottom, _pngExport.margins.bottom());
	WRITE(pngAntialiasing, _pngExport.antialiasing);
	WRITE(pngFileName, _pngExport.fileName);
	settings.endGroup();

	/* Options */
	settings.beginGroup(SETTINGS_OPTIONS);
	WRITE(paletteColor, _options.palette.color());
	WRITE(paletteShift, _options.palette.shift());
	WRITE(mapOpacity, _options.mapOpacity);
	WRITE(backgroundColor, _options.backgroundColor);
	WRITE(crosshairColor, _options.crosshairColor);
	WRITE(infoColor, _options.infoColor);
	WRITE(infoBackground, _options.infoBackground);
	WRITE(trackWidth, _options.trackWidth);
	WRITE(routeWidth, _options.routeWidth);
	WRITE(areaWidth, _options.areaWidth);
	WRITE(trackStyle, (int)_options.trackStyle);
	WRITE(routeStyle, (int)_options.routeStyle);
	WRITE(areaStyle, (int)_options.areaStyle);
	WRITE(areaOpacity, _options.areaOpacity);
	WRITE(waypointSize, _options.waypointSize);
	WRITE(waypointColor, _options.waypointColor);
	WRITE(poiSize, _options.poiSize);
	WRITE(poiColor, _options.poiColor);
	WRITE(graphWidth, _options.graphWidth);
	WRITE(pathAntiAliasing, _options.pathAntiAliasing);
	WRITE(graphAntiAliasing, _options.graphAntiAliasing);
	WRITE(elevationFilter, _options.elevationFilter);
	WRITE(speedFilter, _options.speedFilter);
	WRITE(heartRateFilter, _options.heartRateFilter);
	WRITE(cadenceFilter, _options.cadenceFilter);
	WRITE(powerFilter, _options.powerFilter);
	WRITE(outlierEliminate, _options.outlierEliminate);
	WRITE(detectPauses, _options.detectPauses);
	WRITE(automaticPause, _options.automaticPause);
	WRITE(pauseSpeed, _options.pauseSpeed);
	WRITE(pauseInterval, _options.pauseInterval);
	WRITE(useReportedSpeed, _options.useReportedSpeed);
	WRITE(dataUseDEM, _options.dataUseDEM);
	WRITE(secondaryElevation, _options.showSecondaryElevation);
	WRITE(secondarySpeed, _options.showSecondarySpeed);
	WRITE(timeZone, QVariant::fromValue(_options.timeZone));
	WRITE(useSegments, _options.useSegments);
	WRITE(poiRadius, _options.poiRadius);
	WRITE(demURL, _options.demURL);
	WRITE(demAuthentication, _options.demAuthorization);
	WRITE(demUsername, _options.demUsername);
	WRITE(demPassword, _options.demPassword);
	WRITE(hillshadingAlpha, _options.hillshadingAlpha);
	WRITE(hillshadingLightening, _options.hillshadingLightening);
	WRITE(hillshadingBlur, _options.hillshadingBlur);
	WRITE(hillshadingAzimuth, _options.hillshadingAzimuth);
	WRITE(hillshadingAltitude, _options.hillshadingAltitude);
	WRITE(hillshadingZFactor, _options.hillshadingZFactor);
	WRITE(positionPlugin(), _options.plugin);
	WRITE(positionPluginParameters, _options.pluginParams);
	WRITE(useOpenGL, _options.useOpenGL);
	WRITE(enableHTTP2, _options.enableHTTP2);
	WRITE(pixmapCache, _options.pixmapCache);
	WRITE(demCache, _options.demCache);
	WRITE(connectionTimeout, _options.connectionTimeout);
	WRITE(hiresPrint, _options.hiresPrint);
	WRITE(printName, _options.printName);
	WRITE(printDate, _options.printDate);
	WRITE(printDistance, _options.printDistance);
	WRITE(printTime, _options.printTime);
	WRITE(printMovingTime, _options.printMovingTime);
	WRITE(printItemCount, _options.printItemCount);
	WRITE(separateGraphPage, _options.separateGraphPage);
	WRITE(sliderColor, _options.sliderColor);
	WRITE(outputProjection, _options.outputProjection);
	WRITE(inputProjection, _options.inputProjection);
	WRITE(hidpiMap, _options.hidpiMap);
	WRITE(dataPath, _options.dataPath);
	WRITE(mapsPath, _options.mapsPath);
	WRITE(poiPath, _options.poiPath);
	settings.endGroup();
}

void GUI::readSettings(QString &activeMap, QStringList &disabledPOIs,
  QStringList &recentFiles)
{
#define READ(name) \
	(Settings::name.read(settings))

	QSettings settings(qApp->applicationName(), qApp->applicationName());

#ifndef Q_OS_ANDROID
	settings.beginGroup(SETTINGS_WINDOW);
	restoreGeometry(READ(windowGeometry).toByteArray());
	restoreState(READ(windowState).toByteArray());
	settings.endGroup();
#endif // Q_OS_ANDROID

	/* Settings */
	settings.beginGroup(SETTINGS_SETTINGS);
	TimeType tt = (TimeType)READ(timeType).toInt();
	if (tt == Moving)
		_movingTimeAction->setChecked(true);
	else
		_totalTimeAction->setChecked(true);
	setTimeType(tt);

	Units u = (Units)READ(units).toInt();
	if (u == Imperial)
		_imperialUnitsAction->setChecked(true);
	else if (u == Nautical)
		_nauticalUnitsAction->setChecked(true);
	else
		_metricUnitsAction->setChecked(true);
	setUnits(u);

	CoordinatesFormat cf = (CoordinatesFormat)READ(coordinatesFormat).toInt();
	if (cf == DMS)
		_dmsAction->setChecked(true);
	else if (cf == DegreesMinutes)
		_degreesMinutesAction->setChecked(true);
	else
		_decimalDegreesAction->setChecked(true);
	setCoordinatesFormat(cf);

#ifndef Q_OS_ANDROID
	if (READ(showToolbars).toBool())
		_showToolbarsAction->setChecked(true);
	else
		showToolbars(false);
#endif // Q_OS_ANDROID
	settings.endGroup();

	/* File */
#ifndef Q_OS_ANDROID
	settings.beginGroup(SETTINGS_FILE);
	recentFiles = READ(recentDataFiles);
	settings.endGroup();
#else // Q_OS_ANDROID
	Q_UNUSED(recentFiles);
#endif // Q_OS_ANDROID

	/* Map */
	settings.beginGroup(SETTINGS_MAP);
	if (READ(showMap).toBool()) {
		_showMapAction->setChecked(true);
		_mapView->showMap(true);
	}
	if (READ(cursorCoordinates).toBool()) {
		_showCoordinatesAction->setChecked(true);
		_mapView->showCursorCoordinates(true);
	}
	int layers = READ(layers).toInt();
	if (layers == MapView::Layer::Raster) {
		_drawRastersAction->setChecked(true);
		_mapView->selectLayers(MapView::Layer::Raster);
	} else if (layers == MapView::Layer::Vector) {
		_drawVectorsAction->setChecked(true);
		_mapView->selectLayers(MapView::Layer::Vector);
	} else
		_drawAllAction->setChecked(true);
	activeMap = READ(activeMap).toString();
	settings.endGroup();

	/* Graph */
	settings.beginGroup(SETTINGS_GRAPH);
	if (READ(showGraphs).toBool())
		_showGraphsAction->setChecked(true);
	else
		showGraphs(false);

	GraphType gt = (GraphType)READ(graphType).toInt();
	if (gt == Time)
		_timeGraphAction->setChecked(true);
	else
		_distanceGraphAction->setChecked(true);
	setGraphType(gt);

	if (READ(showGrid).toBool())
		_showGraphGridAction->setChecked(true);
	else
		showGraphGrids(false);

	if (READ(sliderInfo).toBool())
		_showGraphSliderInfoAction->setChecked(true);
	else
		showGraphSliderInfo(false);

#ifdef Q_OS_ANDROID
	if (READ(showGraphTabs).toBool())
		_showGraphTabsAction->setChecked(true);
	else
		showGraphTabs(false);
#endif // Q_OS_ANDROID
	settings.endGroup();

	/* POI */
	settings.beginGroup(SETTINGS_POI);
	if (READ(poiOverlap).toBool()) {
		_overlapPOIAction->setChecked(true);
		_mapView->showOverlappedPOIs(true);
	}
	if (READ(poiIcons).toBool()) {
		_showPOIIconsAction->setChecked(true);
		_mapView->showPOIIcons(true);
	}
	if (READ(poiLabels).toBool()) {
		_showPOILabelsAction->setChecked(true);
		_mapView->showPOILabels(true);
	}
	if (READ(showPoi).toBool()) {
		_showPOIAction->setChecked(true);
		_mapView->showPOI(true);
	}
	disabledPOIs = READ(disabledPoiFiles);
	settings.endGroup();

	/* Data */
	settings.beginGroup(SETTINGS_DATA);
	if (READ(tracks).toBool()) {
		_showTracksAction->setChecked(true);
		_mapView->showTracks(true);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showTracks(true);
	}
	if (READ(routes).toBool()) {
		_showRoutesAction->setChecked(true);
		_mapView->showRoutes(true);
		for (int i = 0; i < _tabs.count(); i++)
			_tabs.at(i)->showRoutes(true);
	}
	if (READ(waypoints).toBool()) {
		_showWaypointsAction->setChecked(true);
		_mapView->showWaypoints(true);
	}
	if (READ(areas).toBool()) {
		_showAreasAction->setChecked(true);
		_mapView->showAreas(true);
	}
	if (READ(waypointIcons).toBool()) {
		_showWaypointIconsAction->setChecked(true);
		_mapView->showWaypointIcons(true);
	}
	if (READ(waypointLabels).toBool()) {
		_showWaypointLabelsAction->setChecked(true);
		_mapView->showWaypointLabels(true);
	}
	if (READ(routeWaypoints).toBool()) {
		_showRouteWaypointsAction->setChecked(true);
		_mapView->showRouteWaypoints(true);
	}
	if (READ(pathTicks).toBool()) {
		_showTicksAction->setChecked(true);
		_mapView->showTicks(true);
	}
	if (READ(useStyles).toBool()) {
		_useStylesAction->setChecked(true);
		_mapView->useStyles(true);
	}
	if (READ(positionMarkers).toBool()) {
		MarkerInfoItem::Type mt = (MarkerInfoItem::Type)READ(markerInfo).toInt();
		if (mt == MarkerInfoItem::Position)
			_showMarkerCoordinatesAction->setChecked(true);
		else if (mt == MarkerInfoItem::Date)
			_showMarkerDateAction->setChecked(true);
		else
			_showMarkersAction->setChecked(true);

		_mapView->showMarkers(true);
		_mapView->showMarkerInfo(mt);
	} else
		_hideMarkersAction->setChecked(true);
	settings.endGroup();

	/* DEM */
	settings.beginGroup(SETTINGS_DEM);
	if (READ(drawHillShading).toBool())
		_drawHillShadingAction->setChecked(true);
	else
		_mapView->drawHillShading(false);
	settings.endGroup();

	/* Position */
	settings.beginGroup(SETTINGS_POSITION);
	if (READ(showPosition).toBool()) {
		_showPositionAction->setChecked(true);
		_mapView->showPosition(true);
	}
	if (READ(followPosition).toBool()) {
		_followPositionAction->setChecked(true);
		_mapView->followPosition(true);
	}
	if (READ(positionCoordinates).toBool()) {
		_showPositionCoordinatesAction->setChecked(true);
		_mapView->showPositionCoordinates(true);
	}
	if (READ(motionInfo).toBool()) {
		_showMotionInfoAction->setChecked(true);
		_mapView->showMotionInfo(true);
	}
	settings.endGroup();

	/* PDF export */
	settings.beginGroup(SETTINGS_PDF_EXPORT);
	_pdfExport.orientation = (QPageLayout::Orientation)READ(pdfOrientation)
	  .toInt();
	_pdfExport.resolution = READ(pdfResolution).toInt();
	_pdfExport.paperSize = (QPageSize::PageSizeId)READ(pdfSize).toInt();
	_pdfExport.margins = QMarginsF(READ(pdfMarginLeft).toReal(),
	  READ(pdfMarginTop).toReal(), READ(pdfMarginRight).toReal(),
	  READ(pdfMarginBottom).toReal());
	_pdfExport.fileName = READ(pdfFileName).toString();
	settings.endGroup();

	/* PNG export */
	settings.beginGroup(SETTINGS_PNG_EXPORT);
	_pngExport.size = QSize(READ(pngWidth).toInt(), READ(pngHeight).toInt());
	_pngExport.margins = QMargins(READ(pngMarginLeft).toInt(),
	  READ(pngMarginTop).toInt(), READ(pngMarginRight).toInt(),
	  READ(pngMarginBottom).toInt());
	_pngExport.antialiasing = READ(pngAntialiasing).toBool();
	_pngExport.fileName = READ(pngFileName).toString();
	settings.endGroup();

	/* Options */
	settings.beginGroup(SETTINGS_OPTIONS);
	_options.palette = Palette(READ(paletteColor).value<QColor>(),
	  READ(paletteShift).toDouble());
	_options.mapOpacity = READ(mapOpacity).toInt();
	_options.backgroundColor = READ(backgroundColor).value<QColor>();
	_options.crosshairColor = READ(crosshairColor).value<QColor>();
	_options.infoColor = READ(infoColor).value<QColor>();
	_options.infoBackground = READ(infoBackground).toBool();
	_options.trackWidth = READ(trackWidth).toInt();
	_options.routeWidth = READ(routeWidth).toInt();
	_options.areaWidth = READ(areaWidth).toInt();
	_options.trackStyle = (Qt::PenStyle)READ(trackStyle).toInt();
	_options.routeStyle = (Qt::PenStyle)READ(routeStyle).toInt();
	_options.areaStyle = (Qt::PenStyle)READ(areaStyle).toInt();
	_options.areaOpacity = READ(areaOpacity).toInt();
	_options.pathAntiAliasing = READ(pathAntiAliasing).toBool();
	_options.waypointSize = READ(waypointSize).toInt();
	_options.waypointColor = READ(waypointColor).value<QColor>();
	_options.poiSize = READ(poiSize).toInt();
	_options.poiColor = READ(poiColor).value<QColor>();
	_options.graphWidth = READ(graphWidth).toInt();
	_options.graphAntiAliasing = READ(graphAntiAliasing).toBool();
	_options.elevationFilter = READ(elevationFilter).toInt();
	_options.speedFilter = READ(speedFilter).toInt();
	_options.heartRateFilter = READ(heartRateFilter).toInt();
	_options.cadenceFilter = READ(cadenceFilter).toInt();
	_options.powerFilter = READ(powerFilter).toInt();
	_options.outlierEliminate = READ(outlierEliminate).toBool();
	_options.pauseSpeed = READ(pauseSpeed).toFloat();
	_options.detectPauses = READ(detectPauses).toBool();
	_options.automaticPause = READ(automaticPause).toBool();
	_options.pauseInterval = READ(pauseInterval).toInt();
	_options.useReportedSpeed = READ(useReportedSpeed).toBool();
	_options.dataUseDEM = READ(dataUseDEM).toBool();
	_options.showSecondaryElevation = READ(secondaryElevation).toBool();
	_options.showSecondarySpeed = READ(secondarySpeed).toBool();
	_options.timeZone = READ(timeZone).value<TimeZoneInfo>();
	_options.useSegments = READ(useSegments).toBool();
	_options.poiRadius = READ(poiRadius).toInt();
	_options.demURL = READ(demURL).toString();
	_options.demAuthorization = READ(demAuthentication).toBool();
	_options.demUsername = READ(demUsername).toString();
	_options.demPassword = READ(demPassword).toString();
	_options.hillshadingAlpha = READ(hillshadingAlpha).toInt();
	_options.hillshadingLightening = READ(hillshadingLightening).toDouble();
	_options.hillshadingBlur = READ(hillshadingBlur).toInt();
	_options.hillshadingAzimuth = READ(hillshadingAzimuth).toInt();
	_options.hillshadingAltitude = READ(hillshadingAltitude).toInt();
	_options.hillshadingZFactor = READ(hillshadingZFactor).toDouble();
	_options.plugin = READ(positionPlugin()).toString();
	_options.pluginParams = READ(positionPluginParameters);
	_options.useOpenGL = READ(useOpenGL).toBool();
	_options.enableHTTP2 = READ(enableHTTP2).toBool();
	_options.pixmapCache = READ(pixmapCache).toInt();
	_options.demCache = READ(demCache).toInt();
	_options.connectionTimeout = READ(connectionTimeout).toInt();
	_options.hiresPrint = READ(hiresPrint).toBool();
	_options.printName = READ(printName).toBool();
	_options.printDate = READ(printDate).toBool();
	_options.printDistance = READ(printDistance).toBool();
	_options.printTime = READ(printTime).toBool();
	_options.printMovingTime = READ(printMovingTime).toBool();
	_options.printItemCount = READ(printItemCount).toBool();
	_options.separateGraphPage = READ(separateGraphPage).toBool();
	_options.sliderColor = READ(sliderColor).value<QColor>();
	_options.outputProjection = READ(outputProjection).toInt();
	_options.inputProjection = READ(inputProjection).toInt();
	_options.hidpiMap = READ(hidpiMap).toBool();
	_options.dataPath = READ(dataPath).toString();
	_options.mapsPath = READ(mapsPath).toString();
	_options.poiPath = READ(poiPath).toString();
	settings.endGroup();

	loadOptions();
}

void GUI::loadOptions()
{
	_positionSource = positionSource(_options);
	_showPositionAction->setEnabled(_positionSource != 0);

	_mapView->setPalette(_options.palette);
	_mapView->setMapOpacity(_options.mapOpacity);
	_mapView->setBackgroundColor(_options.backgroundColor);
	_mapView->setCrosshairColor(_options.crosshairColor);
	_mapView->setInfoColor(_options.infoColor);
	_mapView->drawInfoBackground(_options.infoBackground);
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
	_mapView->useOpenGL(_options.useOpenGL);
	_mapView->setMapConfig(CRS::projection(_options.inputProjection),
	  CRS::projection(4326, _options.outputProjection), _options.hidpiMap);
	_mapView->setTimeZone(_options.timeZone.zone());
	_mapView->setPositionSource(_positionSource);

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
	Track::detectPauses(_options.detectPauses);
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

	Downloader::enableHTTP2(_options.enableHTTP2);
	Downloader::setTimeout(_options.connectionTimeout);

	QPixmapCache::setCacheLimit(_options.pixmapCache * 1024);
	DEM::setCacheSize(_options.demCache * 1024);

	HillShading::setAlpha(_options.hillshadingAlpha);
	HillShading::setBlur(_options.hillshadingBlur);
	HillShading::setAzimuth(_options.hillshadingAzimuth);
	HillShading::setAltitude(_options.hillshadingAltitude);
	HillShading::setZFactor(_options.hillshadingZFactor);
	HillShading::setLightening(_options.hillshadingLightening);

	_poi->setRadius(_options.poiRadius);

	_dem->setUrl(_options.demURL);
	if (_options.demAuthorization)
		_dem->setAuthorization(Authorization(_options.demUsername,
		  _options.demPassword));

	_dataDir = _options.dataPath;
	_mapDir = _options.mapsPath;
	_poiDir = _options.poiPath;
}

void GUI::updateOptions(const Options &options)
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
#define SET_HS_OPTION(option, action) \
	if (options.option != _options.option) { \
		HillShading::action(options.option); \
		redraw = true; \
	}

	bool reload = false;
	bool redraw = false;

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
	SET_VIEW_OPTION(crosshairColor, setCrosshairColor);
	SET_VIEW_OPTION(infoColor, setInfoColor);
	SET_VIEW_OPTION(infoBackground, drawInfoBackground);

	if (options.plugin != _options.plugin
	  || options.pluginParams.value(options.plugin)
	  != _options.pluginParams.value(_options.plugin)) {
		QGeoPositionInfoSource *source = positionSource(options);
		_showPositionAction->setEnabled(source != 0);
		_mapView->setPositionSource(source);
		delete _positionSource;
		_positionSource = source;
	}

	if (options.hidpiMap != _options.hidpiMap
	  || options.outputProjection != _options.outputProjection
	  || options.inputProjection != _options.inputProjection)
		_mapView->setMapConfig(CRS::projection(options.inputProjection),
		  CRS::projection(4326, options.outputProjection), options.hidpiMap);

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
	SET_TRACK_OPTION(detectPauses, detectPauses);
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

	if (options.demURL != _options.demURL)
		_dem->setUrl(options.demURL);
	if (options.demAuthorization != _options.demAuthorization
	  || options.demUsername != _options.demUsername
	  || options.demPassword != _options.demPassword)
		_dem->setAuthorization(options.demAuthorization
		  ? Authorization(options.demUsername, options.demPassword)
		  : Authorization());

	if (options.pixmapCache != _options.pixmapCache)
		QPixmapCache::setCacheLimit(options.pixmapCache * 1024);
	if (options.demCache != _options.demCache)
		DEM::setCacheSize(options.demCache * 1024);

	SET_HS_OPTION(hillshadingAlpha, setAlpha);
	SET_HS_OPTION(hillshadingBlur, setBlur);
	SET_HS_OPTION(hillshadingAzimuth, setAzimuth);
	SET_HS_OPTION(hillshadingAltitude, setAltitude);
	SET_HS_OPTION(hillshadingZFactor, setZFactor);
	SET_HS_OPTION(hillshadingLightening, setLightening);

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
	if (redraw)
		_mapView->setMap(_map);

	_options = options;

	updateDataDEMDownloadAction();
}

void GUI::loadInitialMaps(const QString &selected)
{
	// Load the maps
	QString mapDir(ProgramPaths::mapDir());
	if (mapDir.isNull())
		return;

	TreeNode<Map*> maps(MapList::loadMaps(mapDir, _mapView->inputProjection()));
	createMapNodeMenu(createMapActionsNode(maps), _mapMenu, _mapsEnd);

	// Select the active map according to the user settings
	QAction *ma = mapAction(selected);
	if (ma) {
		ma->trigger();
		_showMapAction->setEnabled(true);
		_clearMapCacheAction->setEnabled(true);
	}
}

void GUI::loadInitialPOIs(const QStringList &disabled)
{
	// Load the POI files
	QString poiDir(ProgramPaths::poiDir());
	if (poiDir.isNull())
		return;

	TreeNode<QString> poiFiles(_poi->loadDir(poiDir));
	createPOINodeMenu(createPOIActionsNode(poiFiles), _poiMenu, _poisEnd);

	// Enable/disable the files according to the user settings
	QList<QAction*> poiActions(_poisActionGroup->actions());
	for (int i = 0; i < poiActions.count(); i++)
		poiActions.at(i)->setChecked(true);
	for (int i = 0; i < disabled.size(); i++) {
		const QString &file = disabled.at(i);
		if (_poi->enableFile(file, false)) {
			for (int j = 0; j < poiActions.size(); j++)
				if (poiActions.at(j)->data().toString() == file)
					poiActions.at(j)->setChecked(false);
		}
	}

	_selectAllPOIAction->setEnabled(!poiActions.isEmpty());
	_unselectAllPOIAction->setEnabled(!poiActions.isEmpty());
}

#ifndef Q_OS_ANDROID
void GUI::loadRecentFiles(const QStringList &files)
{
	QAction *before = _recentFilesEnd;

	for (int i = 0; i < files.size(); i++) {
		QAction *a = new QAction(files.at(i), _recentFilesActionGroup);
		a->setData(files.at(i));
		_recentFilesMenu->insertAction(before, a);
		before = a;
	}

	if (!files.isEmpty())
		_recentFilesMenu->setEnabled(true);
}
#endif // Q_OS_ANDROID

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
	_mapView->setDevicePixelRatio(devicePixelRatioF());

	disconnect(SIGNAL(logicalDotsPerInchChanged(qreal)), this,
	  SLOT(logicalDotsPerInchChanged(qreal)));
	connect(screen, &QScreen::logicalDotsPerInchChanged, this,
	  &GUI::logicalDotsPerInchChanged);
}

void GUI::logicalDotsPerInchChanged(qreal dpi)
{
	Q_UNUSED(dpi)

	_mapView->setDevicePixelRatio(devicePixelRatioF());
}
