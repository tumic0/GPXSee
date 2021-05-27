#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QDate>
#include <QPrinter>
#include "common/treenode.h"
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
class FileBrowser;
class GraphTab;
class MapView;
class Map;
class POI;
class QScreen;
class MapAction;
class POIAction;
class Data;

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI();

	bool openFile(const QString &fileName, bool silent = false);
	bool loadMap(const QString &fileName, MapAction *&action,
	  bool silent = false);
	void show();

private slots:
	void about();
	void keys();
	void paths();
	void printFile();
	void exportPDFFile();
	void exportPNGFile();
	void openFile();
	void closeAll();
	void reloadFiles();
	void statistics();
	void openPOIFile();
	void showGraphs(bool show);
	void showGraphGrids(bool show);
	void showGraphSliderInfo(bool show);
	void showPathMarkerInfo(QAction *action);
	void showToolbars(bool show);
	void showFullscreen(bool show);
	void showTracks(bool show);
	void showRoutes(bool show);
	void loadMap();
	void loadMapDir();
	void nextMap();
	void prevMap();
	void openOptions();
	void clearMapCache();

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

private:
	typedef QPair<QDateTime, QDateTime> DateTimeRange;

	void closeFiles();
	void plot(QPrinter *printer);
	void plotMainPage(QPainter *painter, const QRectF &rect, qreal ratio,
	  bool expand = false);
	void plotGraphsPage(QPainter *painter, const QRectF &rect, qreal ratio);
	qreal graphPlotHeight(const QRectF &rect, qreal ratio);

	TreeNode<POIAction*> createPOIActions();
	TreeNode<POIAction*> createPOIActionsNode(const TreeNode<QString> &node);
	TreeNode<MapAction*> createMapActions();
	TreeNode<MapAction*> createMapActionsNode(const TreeNode<Map*> &node);
	void createActions(TreeNode<MapAction*> &mapActions,
	  TreeNode<POIAction*> &poiActions);
	void createMapNodeMenu(const TreeNode<MapAction*> &node, QMenu *menu);
	void createPOINodeMenu(const TreeNode<POIAction*> &node, QMenu *menu);
	void createMenus(const TreeNode<MapAction*> &mapActions,
	  const TreeNode<POIAction*> &poiActions);
	void createToolBars();
	void createStatusBar();
	void createMapView();
	void createGraphTabs();
	void createBrowser();

	bool openPOIFile(const QString &fileName);
	bool loadFile(const QString &fileName, bool silent = false);
	void loadData(const Data &data);
	bool loadMapNode(const TreeNode<Map*> &node, MapAction *&action,
	  bool silent, const QList<QAction*> &existingActions);
	void loadMapDirNode(const TreeNode<Map*> &node, QList<MapAction*> &actions,
	  QMenu *menu, const QList<QAction*> &existingActions);
	void updateStatusBarInfo();
	void updateWindowTitle();
	bool updateGraphTabs();

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
	void readSettings();
	void writeSettings();

	void keyPressEvent(QKeyEvent *event);
	void closeEvent(QCloseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	QToolBar *_fileToolBar;
	QToolBar *_showToolBar;
	QToolBar *_navigationToolBar;
	QMenu *_poiMenu;
	QMenu *_mapMenu;

	QActionGroup *_fileActionGroup;
	QActionGroup *_navigationActionGroup;
	QActionGroup *_mapsActionGroup;
	QActionGroup *_poisActionGroup;
	QAction *_exitAction;
	QAction *_keysAction;
	QAction *_pathsAction;
	QAction *_aboutAction;
	QAction *_aboutQtAction;
	QAction *_printFileAction;
	QAction *_exportPDFFileAction;
	QAction *_exportPNGFileAction;
	QAction *_openFileAction;
	QAction *_closeFileAction;
	QAction *_reloadFileAction;
	QAction *_statisticsAction;
	QAction *_openPOIAction;
	QAction *_selectAllPOIAction;
	QAction *_unselectAllPOIAction;
	QAction *_showPOIAction;
	QAction *_overlapPOIAction;
	QAction *_showPOILabelsAction;
	QAction *_showMapAction;
	QAction *_fullscreenAction;
	QAction *_loadMapAction;
	QAction *_loadMapDirAction;
	QAction *_clearMapCacheAction;
	QAction *_showGraphsAction;
	QAction *_showGraphGridAction;
	QAction *_showGraphSliderInfoAction;
	QAction *_distanceGraphAction;
	QAction *_timeGraphAction;
	QAction *_showToolbarsAction;
	QAction *_nextAction;
	QAction *_prevAction;
	QAction *_lastAction;
	QAction *_firstAction;
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
	QAction *_showAreasAction;
	QAction *_showRouteWaypointsAction;
	QAction *_hideMarkersAction;
	QAction *_showMarkersAction;
	QAction *_showMarkerDateAction;
	QAction *_showMarkerCoordinatesAction;
	QAction *_showTicksAction;
	QAction *_showCoordinatesAction;
	QAction *_openOptionsAction;
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

	FileBrowser *_browser;
	QList<QString> _files;

	int _trackCount, _routeCount, _areaCount, _waypointCount;
	qreal _trackDistance, _routeDistance;
	qreal _time, _movingTime;
	DateTimeRange _dateRange;
	QString _pathName;

	QList<QByteArray> _windowStates;
	QList<QByteArray> _windowGeometries;
	int _frameStyle;

	PDFExport _pdfExport;
	PNGExport _pngExport;
	Options _options;

	QString _dataDir, _mapDir, _poiDir;

	Units _units;
};

#endif // GUI_H
