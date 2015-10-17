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

class ElevationGraph;
class SpeedGraph;
class Track;

class GUI : public QMainWindow
{
	Q_OBJECT

public:
	GUI();

	bool openFile(const QString &fileName);
	void setDir(const QString &file);

private slots:
	void about();
	void saveFile();
	void saveAs();
	void openFile();
	void closeFile();
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

	void saveFile(const QString &fileName);
	void updateStatusBarInfo(const QString &fileName);

	void keyPressEvent(QKeyEvent * event);

	QMenu *_fileMenu;
	QMenu *_aboutMenu;
	QMenu *_poiMenu;

	QToolBar *_fileToolBar;
	QToolBar *_poiToolBar;
	QTabWidget *_trackGraphs;
	QActionGroup *_fileActionGroup;

	QAction *_exitAction;
	QAction *_aboutAction;
	QAction *_aboutQtAction;
	QAction *_saveFileAction;
	QAction *_saveAsAction;
	QAction *_openFileAction;
	QAction *_closeFileAction;
	QAction *_openPOIAction;
	QAction *_showPOIAction;

	QLabel *_fileNameLabel;
	QLabel *_distanceLabel;
	QLabel *_timeLabel;

	ElevationGraph *_elevationGraph;
	SpeedGraph *_speedGraph;
	Track *_track;

	POI _poi;

	QFileInfoList _dirFiles;
	int _dirIndex;

	QString _saveFileName;
	unsigned _files;

	qreal _distance;
	qreal _time;
};

#endif // GUI_H
