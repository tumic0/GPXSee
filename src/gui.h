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

class Graph;
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

	void keyPressEvent(QKeyEvent * event);

	void saveFile(const QString &fileName);

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

	QLabel *_fileName;
	QLabel *_zoom;

	Graph *_elevationGraph;
	Graph *_speedGraph;
	Track *_track;

	POI _poi;

	QFileInfoList _dirFiles;
	int _dirIndex;

	QString _saveFileName;
};

#endif // GUI_H
