#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QDate>
#include <QPrinter>
#include "common/treenode.h"
#include "common/rectc.h"
#include "data/graph.h"
#include "units.h"
#include "timetype.h"
#include "format.h"
#include "pdfexportdialog.h"
#include "pngexportdialog.h"
#include "optionsdialog.h"

class QMenu;
class QToolBar;
class QTabWidget;
class QActionGroup;
class QAction;
class QLabel;
class QSplitter;
class QPrinter;
class QGeoPositionInfoSource;
class FileBrowser;
class GraphTab;
class MapView;
class Map;
class POI;
class QScreen;
class MapAction;
class POIAction;
class Data;
class DEMLoader;
class NavigationWidget;

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI();

	bool openFile(const QString &fileName, bool tryUnknown, int &showError);
	bool loadMap(const QString &fileName, MapAction *&action, int &showError);
	void show();
	void writeSettings();

private slots:
	void about();
#ifndef Q_OS_ANDROID
	void keys();
#endif // Q_OS_ANDROID
	void paths();
	void printFile();
	void exportPDFFile();
	void exportPNGFile();
	void openFile();
#ifdef Q_OS_ANDROID
	void openDir();
#endif // Q_OS_ANDROID
	void closeAll();
	void reloadFiles();
	void statistics();
	void openPOIFile();
	void showGraphs(bool show);
	void showGraphGrids(bool show);
	void showGraphSliderInfo(bool show);
	void showPathMarkerInfo(QAction *action);
#ifdef Q_OS_ANDROID
	void showGraphTabs(bool show);
#else // Q_OS_ANDROID
	void showToolbars(bool show);
	void showFullscreen(bool show);
#endif // Q_OS_ANDROID
	void showTracks(bool show);
	void showRoutes(bool show);
	void showAreas(bool show);
	void showWaypoints(bool show);
	void loadMap();
	void loadMapDir();
	void nextMap();
	void prevMap();
	void openOptions();
	void clearMapCache();
	void downloadDEM();
	void showDEMTiles();

	void mapChanged(QAction *action);
	void graphChanged(int);
	void poiFileChecked(QAction *action);
	void selectAllPOIs();
	void unselectAllPOIs();

	void next();
	void prev();
	void last();
	void first();
	void updateNavigationActions();

	void setTotalTime() {setTimeType(Total);}
	void setMovingTime() {setTimeType(Moving);}
	void setMetricUnits() {setUnits(Metric);}
	void setImperialUnits() {setUnits(Imperial);}
	void setNauticalUnits() {setUnits(Nautical);}
	void setDistanceGraph() {setGraphType(Distance);}
	void setTimeGraph() {setGraphType(Time);}
	void setDecimalDegrees() {setCoordinatesFormat(DecimalDegrees);}
	void setDegreesMinutes() {setCoordinatesFormat(DegreesMinutes);}
	void setDMS() {setCoordinatesFormat(DMS);}

	void screenChanged(QScreen *screen);
	void logicalDotsPerInchChanged(qreal dpi);

	void mapLoaded();
	void mapLoadedDir();
	void mapInitialized();

	void demLoaded();

private:
	typedef QPair<QDateTime, QDateTime> DateTimeRange;

	void closeFiles();
	void plot(QPrinter *printer);
	void plotMainPage(QPainter *painter, const QRectF &rect, qreal ratio,
	  bool expand = false);
	void plotGraphsPage(QPainter *painter, const QRectF &rect, qreal ratio);
	qreal graphPlotHeight(const QRectF &rect, qreal ratio);

	TreeNode<POIAction*> createPOIActionsNode(const TreeNode<QString> &node);
	TreeNode<MapAction*> createMapActionsNode(const TreeNode<Map*> &node);
	void createMapNodeMenu(const TreeNode<MapAction*> &node, QMenu *menu,
	  QAction *action = 0);
	void createPOINodeMenu(const TreeNode<POIAction*> &node, QMenu *menu,
	  QAction *action = 0);
	void createActions();
	void createMenus();
#ifdef Q_OS_ANDROID
	void createNavigation();
#else // Q_OS_ANDROID
	void createToolBars();
#endif // Q_OS_ANDROID
	void createStatusBar();
	void createMapView();
	void createGraphTabs();
	void createBrowser();

	bool openPOIFile(const QString &fileName);
	bool loadFile(const QString &fileName, bool tryUnknown, int &showError);
	void loadData(const Data &data);
	bool loadMapNode(const TreeNode<Map*> &node, MapAction *&action,
	  const QList<QAction*> &existingActions, int &showError);
	void loadMapDirNode(const TreeNode<Map*> &node, QList<MapAction*> &actions,
	  QMenu *menu, const QList<QAction*> &existingActions, int &showError);
	void updateStatusBarInfo();
	void updateWindowTitle();
	bool updateGraphTabs();
	void updateDEMDownloadAction();

	TimeType timeType() const;
	Units units() const;
	void setTimeType(TimeType type);
	void setUnits(Units units);
	void setCoordinatesFormat(CoordinatesFormat format);
	void setGraphType(GraphType type);

	qreal distance() const;
	qreal time() const;
	qreal movingTime() const;
	QAction *mapAction(const QString &name);
	QGeoPositionInfoSource *positionSource(const Options &options);
	void readSettings(QString &activeMap, QStringList &disabledPOIs);

	void loadInitialMaps(const QString &selected);
	void loadInitialPOIs(const QStringList &disabled);

	void loadOptions();
	void updateOptions(const Options &options);

#ifndef Q_OS_ANDROID
	void keyPressEvent(QKeyEvent *event);
#endif // Q_OS_ANDROID
	void closeEvent(QCloseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

#ifdef Q_OS_ANDROID
	NavigationWidget *_navigation;
#else // Q_OS_ANDROID
	QToolBar *_fileToolBar;
	QToolBar *_showToolBar;
	QToolBar *_navigationToolBar;
#endif // Q_OS_ANDROID
	QMenu *_poiMenu;
	QMenu *_mapMenu;

	QActionGroup *_fileActionGroup;
	QActionGroup *_navigationActionGroup;
	QActionGroup *_mapsActionGroup;
	QActionGroup *_poisActionGroup;
#if !defined(Q_OS_MAC) && !defined(Q_OS_ANDROID)
	QAction *_exitAction;
#endif // Q_OS_MAC + Q_OS_ANDROID
	QAction *_pathsAction;
	QAction *_aboutAction;
	QAction *_printFileAction;
	QAction *_exportPDFFileAction;
	QAction *_exportPNGFileAction;
	QAction *_openFileAction;
	QAction *_openDirAction;
	QAction *_closeFileAction;
	QAction *_reloadFileAction;
	QAction *_statisticsAction;
	QAction *_openPOIAction;
	QAction *_selectAllPOIAction;
	QAction *_unselectAllPOIAction;
	QAction *_showPOIAction;
	QAction *_overlapPOIAction;
	QAction *_showPOILabelsAction;
	QAction *_showPOIIconsAction;
	QAction *_showMapAction;
	QAction *_showPositionAction;
	QAction *_followPositionAction;
	QAction *_showPositionCoordinatesAction;
	QAction *_showMotionInfoAction;
	QAction *_loadMapAction;
	QAction *_loadMapDirAction;
	QAction *_clearMapCacheAction;
	QAction *_showGraphsAction;
	QAction *_showGraphGridAction;
	QAction *_showGraphSliderInfoAction;
	QAction *_distanceGraphAction;
	QAction *_timeGraphAction;
#ifdef Q_OS_ANDROID
	QAction *_showGraphTabsAction;
#else // Q_OS_ANDROID
	QAction *_keysAction;
	QAction *_fullscreenAction;
	QAction *_showToolbarsAction;
	QAction *_nextAction;
	QAction *_prevAction;
	QAction *_lastAction;
	QAction *_firstAction;
#endif // Q_OS_ANDROID
	QAction *_metricUnitsAction;
	QAction *_imperialUnitsAction;
	QAction *_nauticalUnitsAction;
	QAction *_decimalDegreesAction;
	QAction *_degreesMinutesAction;
	QAction *_dmsAction;
	QAction *_totalTimeAction;
	QAction *_movingTimeAction;
	QAction *_nextMapAction;
	QAction *_prevMapAction;
	QAction *_showTracksAction;
	QAction *_showRoutesAction;
	QAction *_showWaypointsAction;
	QAction *_showWaypointLabelsAction;
	QAction *_showWaypointIconsAction;
	QAction *_showAreasAction;
	QAction *_showRouteWaypointsAction;
	QAction *_hideMarkersAction;
	QAction *_showMarkersAction;
	QAction *_showMarkerDateAction;
	QAction *_showMarkerCoordinatesAction;
	QAction *_showTicksAction;
	QAction *_useStylesAction;
	QAction *_showCoordinatesAction;
	QAction *_openOptionsAction;
	QAction *_downloadDEMAction;
	QAction *_showDEMTilesAction;
	QAction *_mapsEnd;
	QAction *_poisEnd;

	QLabel *_fileNameLabel;
	QLabel *_distanceLabel;
	QLabel *_timeLabel;

	QSplitter *_splitter;
	MapView *_mapView;
	QTabWidget *_graphTabWidget;
	QList<GraphTab*> _tabs;
	GraphTab *_lastTab;

	POI *_poi;
	Map *_map;
	QGeoPositionInfoSource *_positionSource;
	DEMLoader *_dem;

	FileBrowser *_browser;
	QList<QString> _files;

	int _trackCount, _routeCount, _areaCount, _waypointCount;
	qreal _trackDistance, _routeDistance;
	qreal _time, _movingTime;
	DateTimeRange _dateRange;
	QString _pathName;

#ifndef Q_OS_ANDROID
	QList<QByteArray> _windowStates;
	QList<QByteArray> _windowGeometries;
	int _frameStyle;
#endif // Q_OS_ANDROID

	PDFExport _pdfExport;
	PNGExport _pngExport;
	Options _options;

	QString _dataDir, _mapDir, _poiDir;

	Units _units;

	QList<RectC> _demRects;
};

#endif // GUI_H
