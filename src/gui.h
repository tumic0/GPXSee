#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QDate>
#include <QPrinter>
#include "poi.h"
#include "margins.h"

class QMenu;
class QToolBar;
class QTabWidget;
class QActionGroup;
class QAction;
class QLabel;
class QSignalMapper;
class QPrinter;
class FileBrowser;
class GraphView;
class ElevationGraph;
class SpeedGraph;
class HeartRateGraph;
class TemperatureGraph;
class TrackView;
class Map;

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI(QWidget *parent = 0);
	~GUI();

	bool openFile(const QString &fileName);

private slots:
	void about();
	void keys();
	void dataSources();
	void printFile();
	void exportFile();
	void openFile();
	void closeAll();
	void reloadFile();
	void openPOIFile();
	void closePOIFiles();
	void showPOI(bool checked);
	void showMap(bool checked);
	void showGraphs(bool checked);
	void showToolbars(bool checked);
	void showFullscreen(bool checked);
	void clearMapCache();
	void nextMap();
	void prevMap();

	void mapChanged(int);
	void graphChanged(int);
	void poiFileChecked(int);

	void next();
	void prev();
	void last();
	void first();

	void setMetricUnits();
	void setImperialUnits();

	void sliderPositionChanged(qreal pos);

private:
	typedef QPair<GraphView *, QString> GraphTab;
	typedef QPair<QDate, QDate> DateRange;

	void loadMaps();
	void loadPOIs();
	void closeFiles();
	void plot(QPrinter *printer);

	QAction *createPOIFileAction(int index);
	void createPOIFilesActions();
	void createMapActions();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createTrackView();
	void createTrackGraphs();

	bool openPOIFile(const QString &fileName);
	bool loadFile(const QString &fileName);
	void exportFile(const QString &fileName);
	void updateStatusBarInfo();
	void updateWindowTitle();
	void updateNavigationActions();
	void updateGraphTabs();
	void updateTrackView();

	void keyPressEvent(QKeyEvent * event);
	void closeEvent(QCloseEvent *event);

	int mapIndex(const QString &name);
	void readSettings();
	void writeSettings();

	QMenu *_fileMenu;
	QMenu *_helpMenu;
	QMenu *_poiMenu;
	QMenu *_mapMenu;
	QMenu *_settingsMenu;
	QMenu *_unitsMenu;
	QMenu *_poiFilesMenu;

	QToolBar *_fileToolBar;
	QToolBar *_showToolBar;
	QToolBar *_navigationToolBar;
	QTabWidget *_trackGraphs;
	QActionGroup *_fileActionGroup;
	QActionGroup *_navigationActionGroup;

	QAction *_exitAction;
	QAction *_keysAction;
	QAction *_dataSourcesAction;
	QAction *_aboutAction;
	QAction *_aboutQtAction;
	QAction *_printFileAction;
	QAction *_exportFileAction;
	QAction *_openFileAction;
	QAction *_closeFileAction;
	QAction *_reloadFileAction;
	QAction *_openPOIAction;
	QAction *_closePOIAction;
	QAction *_showPOIAction;
	QAction *_showMapAction;
	QAction *_fullscreenAction;
	QAction *_clearMapCacheAction;
	QAction *_showGraphsAction;
	QAction *_showToolbarsAction;
	QAction *_nextAction;
	QAction *_prevAction;
	QAction *_lastAction;
	QAction *_firstAction;
	QAction *_metricUnitsAction;
	QAction *_imperialUnitsAction;
	QAction *_nextMapAction;
	QAction *_prevMapAction;
	QList<QAction*> _mapActions;
	QList<QAction*> _poiFilesActions;

	QSignalMapper *_poiFilesSM;

	QLabel *_fileNameLabel;
	QLabel *_distanceLabel;
	QLabel *_timeLabel;

	TrackView *_track;
	ElevationGraph *_elevationGraph;
	SpeedGraph *_speedGraph;
	HeartRateGraph *_heartRateGraph;
	TemperatureGraph *_temperatureGraph;
	QList<GraphTab> _tabs;

	POI _poi;
	QList<Map*> _maps;

	FileBrowser *_browser;
	QList<QString> _files;
	Map *_currentMap;

	int _trackCount;
	qreal _distance;
	qreal _time;
	DateRange _dateRange;

	qreal _sliderPos;

	int _frameStyle;
	bool _showGraphs;

	QString _exportFileName;
	QPrinter::PaperSize _exportPaperSize;
	QPrinter::Orientation _exportOrientation;
	MarginsF _exportMargins;
};

#endif // GUI_H
