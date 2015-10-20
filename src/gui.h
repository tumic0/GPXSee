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
#include <QFileInfoList>
#include "poi.h"


class FileBrowser;
class ElevationGraph;
class SpeedGraph;
class Track;

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
	void showPOI();

	void graphChanged(int);

private:
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

	QToolBar *_fileToolBar;
	QToolBar *_poiToolBar;
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

	QLabel *_fileNameLabel;
	QLabel *_distanceLabel;
	QLabel *_timeLabel;

	ElevationGraph *_elevationGraph;
	SpeedGraph *_speedGraph;
	Track *_track;

	POI _poi;

	FileBrowser *_browser;
	QList<QString> _files;
	QString _saveFileName;

	qreal _distance;
	qreal _time;
};

#endif // GUI_H
