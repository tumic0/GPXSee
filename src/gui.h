#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QTabWidget>
#include <QGraphicsView>
#include <QActionGroup>
#include <QAction>
#include <QLabel>
#include "poi.h"


class FileBrowser;
class ElevationGraph;
class SpeedGraph;
class Track;
class Map;

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI();

	bool openFile(const QString &fileName);

private slots:
	void about();
	void keys();
	void saveFile();
	void saveAs();
	void openFile();
	void closeFile();
	void reloadFile();
	void openPOIFile();
	void showPOI(bool checked);
	void showMap(bool checked);
	void showGraphs(bool checked);
	void showToolbars(bool checked);

	void mapChanged(int);
	void graphChanged(int);

private:
	void loadFiles();

	void createMapActions();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createTrackView();
	void createTrackGraphs();

	bool loadFile(const QString &fileName);
	void saveFile(const QString &fileName);
	void updateStatusBarInfo();

	void keyPressEvent(QKeyEvent * event);

	QMenu *_fileMenu;
	QMenu *_helpMenu;
	QMenu *_poiMenu;
	QMenu *_mapMenu;
	QMenu *_settingsMenu;

	QToolBar *_fileToolBar;
	QToolBar *_showToolBar;
	QTabWidget *_trackGraphs;
	QActionGroup *_fileActionGroup;

	QAction *_exitAction;
	QAction *_keysAction;
	QAction *_aboutAction;
	QAction *_aboutQtAction;
	QAction *_saveFileAction;
	QAction *_saveAsAction;
	QAction *_openFileAction;
	QAction *_closeFileAction;
	QAction *_reloadFileAction;
	QAction *_openPOIAction;
	QAction *_showPOIAction;
	QAction *_showMapAction;
	QAction *_showGraphsAction;
	QAction *_showToolbarsAction;
	QList<QAction*> _mapActions;

	QLabel *_fileNameLabel;
	QLabel *_distanceLabel;
	QLabel *_timeLabel;

	ElevationGraph *_elevationGraph;
	SpeedGraph *_speedGraph;
	Track *_track;

	POI _poi;
	QList<Map*> _maps;

	FileBrowser *_browser;
	QList<QString> _files;
	QString _saveFileName;
	Map *_currentMap;

	qreal _distance;
	qreal _time;
};

#endif // GUI_H
