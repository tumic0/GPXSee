#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QDate>
#include <QPrinter>
#include <QFileSystemWatcher>
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
class QSignalMapper;
class QPrinter;
class FileBrowser;
class GraphTab;
class MapView;
class Map;
class POI;
class QScreen;
class MapAction;
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
	void closePOIFiles();
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

	void mapChanged();
	void graphChanged(int);
	void poiFileChecked(int);
	void fileChanged(const QString & path);

	void next();
	void prev();
	void last();
	void first();

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

	void loadPOIs();
	void closeFiles();
	void plot(QPrinter *printer);
	void plotMainPage(QPainter *painter, const QRectF &rect, qreal ratio,
	  bool expand = false);
	void plotGraphsPage(QPainter *painter, const QRectF &rect, qreal ratio);
	qreal graphPlotHeight(const QRectF &rect, qreal ratio);

	QAction *createPOIFileAction(const QString &fileName);
	MapAction *createMapAction(Map *map);
	void createPOIFilesActions();
	void createMapActions();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createMapView();
	void createGraphTabs();
	void createBrowser();

	bool openPOIFile(const QString &fileName);
	bool loadFile(const QString &fileName, bool silent = false);
	void loadData(const Data &data);
	void updateStatusBarInfo();
	void updateWindowTitle();
	void updateNavigationActions();
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
	QMenu *_poiFilesMenu;
	QMenu *_mapMenu;

	QFileSystemWatcher * _fileWatcher;
	QActionGroup *_fileActionGroup;
	QActionGroup *_navigationActionGroup;
	QActionGroup *_mapsActionGroup;
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
	QAction *_closePOIAction;
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

	QList<QAction*> _poiFilesActions;
	QSignalMapper *_poiFilesSignalMapper;

	QLabel *_fileNameLabel;
	QLabel *_distanceLabel;
	QLabel *_timeLabel;

	QSplitter *_splitter;
	MapView *_mapView;
	QTabWidget *_graphTabWidget;
	QList<GraphTab*> _tabs;

	POI *_poi;
	Map *_map;

	FileBrowser *_browser;
	QList<QString> _files;

	int _trackCount, _routeCount, _areaCount, _waypointCount;
	qreal _trackDistance, _routeDistance;
	qreal _time, _movingTime;
	DateTimeRange _dateRange;
	QString _pathName;

	GraphTab *_lastGraphTab;

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
