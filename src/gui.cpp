#include <QApplication>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPainter>
#include <QKeyEvent>
#include <QDir>
#include "gui.h"
#include "config.h"
#include "icons.h"
#include "keys.h"
#include "gpx.h"
#include "graph.h"
#include "track.h"

#include <QDebug>


GUI::GUI()
{
	createActions();
	createMenus();
	createToolBars();
	createTrackView();
	createTrackGraphs();
	createStatusBar();

	connect(_elevationGraph, SIGNAL(sliderPositionChanged(qreal)), _track,
	  SLOT(movePositionMarker(qreal)));
	connect(_speedGraph, SIGNAL(sliderPositionChanged(qreal)), _track,
	  SLOT(movePositionMarker(qreal)));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(_track);
	layout->addWidget(_trackGraphs);

	QWidget *widget = new QWidget;
	widget->setLayout(layout);
	setCentralWidget(widget);

	setWindowTitle(APP_NAME);
	setUnifiedTitleAndToolBarOnMac(true);

	_dirIndex = -1;

	resize(600, 800);
}

void GUI::createActions()
{
	// Action Groups
	_fileActionGroup = new QActionGroup(this);
	_fileActionGroup->setExclusive(false);
	_fileActionGroup->setEnabled(false);


	// General actions
	_exitAction = new QAction(QIcon(QPixmap(QUIT_ICON)), tr("Quit"), this);
	_exitAction->setShortcut(QKeySequence::Quit);
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	_aboutAction = new QAction(QIcon(QPixmap(APP_ICON)),
	  tr("About GPXSee"), this);
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));
	_aboutQtAction = new QAction(tr("About Qt"), this);
	connect(_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// File related actions
	_openFileAction = new QAction(QIcon(QPixmap(OPEN_FILE_ICON)),
	  tr("Open"), this);
	_openFileAction->setShortcut(QKeySequence::Open);
	connect(_openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));
	_saveFileAction = new QAction(QIcon(QPixmap(SAVE_FILE_ICON)),
	  tr("Save"), this);
	_saveFileAction->setShortcut(QKeySequence::Save);
	_saveFileAction->setActionGroup(_fileActionGroup);
	connect(_saveFileAction, SIGNAL(triggered()), this, SLOT(saveFile()));
	_saveAsAction = new QAction(QIcon(QPixmap(SAVE_AS_ICON)),
	  tr("Save as"), this);
	_saveAsAction->setShortcut(QKeySequence::SaveAs);
	_saveAsAction->setActionGroup(_fileActionGroup);
	connect(_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
	_closeFileAction = new QAction(QIcon(QPixmap(CLOSE_FILE_ICON)),
	  tr("Close"), this);
	_closeFileAction->setShortcut(QKeySequence::Close);
	_closeFileAction->setActionGroup(_fileActionGroup);
	connect(_closeFileAction, SIGNAL(triggered()), this, SLOT(closeFile()));

	// POI actions
	_openPOIAction = new QAction(QIcon(QPixmap(OPEN_FILE_ICON)),
	  tr("Load file"), this);
	connect(_openPOIAction, SIGNAL(triggered()), this, SLOT(openPOIFile()));
	_showPOIAction = new QAction(QIcon(QPixmap(SHOW_POI_ICON)),
	  tr("Show"), this);
	_showPOIAction->setCheckable(true);
	connect(_showPOIAction, SIGNAL(triggered()), this, SLOT(showPOI()));
}

void GUI::createMenus()
{
	_fileMenu = menuBar()->addMenu(tr("File"));
	_fileMenu->addAction(_openFileAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_saveFileAction);
	_fileMenu->addAction(_saveAsAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_closeFileAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_poiMenu = menuBar()->addMenu(tr("POI"));
	_poiMenu->addAction(_openPOIAction);
	_poiMenu->addAction(_showPOIAction);

	_aboutMenu = menuBar()->addMenu(tr("Help"));
	_aboutMenu->addAction(_aboutAction);
	_aboutMenu->addAction(_aboutQtAction);
}

void GUI::createToolBars()
{
	_fileToolBar = addToolBar(tr("File"));
	_fileToolBar->addAction(_openFileAction);
	_fileToolBar->addAction(_saveFileAction);
	_fileToolBar->addAction(_closeFileAction);
#ifdef __APPLE__
	_fileToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // __APPLE__

	_poiToolBar = addToolBar(tr("POI"));
	_poiToolBar->addAction(_showPOIAction);
#ifdef __APPLE__
	_poiToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif // __APPLE__
}

void GUI::createTrackView()
{
	_track = new Track(this);
}

void GUI::createTrackGraphs()
{
	_elevationGraph = new Graph;
	_elevationGraph->setXLabel(tr("Distance [km]"));
	_elevationGraph->setYLabel(tr("Elevation [m.a.s.l.]"));
	_elevationGraph->setXScale(0.001);

	_speedGraph = new Graph;
	_speedGraph->setXLabel(tr("Distance [km]"));
	_speedGraph->setYLabel(tr("Speed [km/h]"));
	_speedGraph->setXScale(0.001);
	_speedGraph->setYScale(3.6);

	_trackGraphs = new QTabWidget;
	_trackGraphs->addTab(_elevationGraph, tr("Elevation"));
	_trackGraphs->addTab(_speedGraph, tr("Speed"));
	connect(_trackGraphs, SIGNAL(currentChanged(int)), this,
	  SLOT(graphChanged(int)));

	_trackGraphs->setFixedHeight(200);
	_trackGraphs->setSizePolicy(
		QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
}

void GUI::createStatusBar()
{
	_fileName = new QLabel();
	_zoom = new QLabel();
	_zoom->setAlignment(Qt::AlignHCenter);

	statusBar()->addPermanentWidget(_fileName, 9);
	statusBar()->addPermanentWidget(_zoom, 1);
	statusBar()->setSizeGripEnabled(false);
}

void GUI::about()
{
	QMessageBox::about(this, tr("About GPXSee"),
	  QString("<h3>") + QString(APP_NAME" "APP_VERSION)
	  + QString("</h3><p>") + tr("GPX viewer and analyzer") + QString("<p/>")
	  + QString("<p>") + tr("GPXSee is distributed under the terms of the "
	  "GNU General Public License version 3. For more info about GPXSee visit "
	  "the project homepage at ")
	  + QString("<a href=\""APP_HOMEPAGE"\">"APP_HOMEPAGE"</a>.</p>"));
}

void GUI::openFile()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Open file"));
	QStringList list = files;
	QStringList::Iterator it = list.begin();

	while(it != list.end()) {
		openFile(*it);
		++it;
	}

	if (!list.empty())
		setDir(list.back());
}

bool GUI::openFile(const QString &fileName)
{
	GPX gpx;

	if (!fileName.isEmpty()) {
		if (gpx.loadFile(fileName)) {
			_elevationGraph->loadData(gpx.elevationGraph());
			_speedGraph->loadData(gpx.speedGraph());
			_track->loadData(gpx.track());
			if (_showPOIAction->isChecked())
				_track->loadPOI(_poi);

			_fileActionGroup->setEnabled(true);
			_fileName->setText(fileName);

			return true;
		} else {
			QMessageBox::critical(this, tr("Error"), fileName + QString("\n\n")
			  + tr("Error loading GPX file:\n%1").arg(gpx.errorString()));
		}
	}

	return false;
}

void GUI::openPOIFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open POI file"));

	if (!fileName.isEmpty()) {
		if (!_poi.loadFile(fileName)) {
			QMessageBox::critical(this, tr("Error"),
			  tr("Error loading POI file:\n%1").arg(_poi.errorString()));
		} else {
			_showPOIAction->setChecked(true);
			_track->loadPOI(_poi);
		}
	}
}

void GUI::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Export to PDF",
	  QString(), "*.pdf");

	if (!fileName.isEmpty()) {
		saveFile(fileName);
		_saveFileName = fileName;
	}
}

void GUI::saveFile()
{
	if (_saveFileName.isEmpty())
		emit saveAs();
	else
		saveFile(_saveFileName);
}

void GUI::saveFile(const QString &fileName)
{
	QPrinter printer(QPrinter::HighResolution);
	printer.setPageSize(QPrinter::A4);
	printer.setOrientation(_track->orientation());
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(fileName);

	QPainter p(&printer);
	int margin = (printer.paperRect().height() - printer.pageRect().height())
	  / 2;
	_track->plot(&p, QRectF(0, 0, printer.width(), (0.80 * printer.height())
	  - margin));
	_elevationGraph->plot(&p,  QRectF(0, 0.80 * printer.height(),
	  printer.width(), printer.height() * 0.20));
	p.end();
}

void GUI::closeFile()
{
	_elevationGraph->clear();
	_speedGraph->clear();
	_track->clear();
	_fileName->clear();

	_fileActionGroup->setEnabled(false);
}

void GUI::showPOI()
{
	if (_showPOIAction->isChecked())
		_track->loadPOI(_poi);
	else
		_track->clearPOI();
}

void GUI::graphChanged(int index)
{
	if (_trackGraphs->widget(index) == _elevationGraph)
		_elevationGraph->setSliderPosition(_speedGraph->sliderPosition());
	else if (_trackGraphs->widget(index) == _speedGraph)
		_speedGraph->setSliderPosition(_elevationGraph->sliderPosition());
}


void GUI::keyPressEvent(QKeyEvent *event)
{
	if (_dirIndex < 0 || _dirFiles.count() == 1)
		return;

	if (event->key() == PREV_KEY) {
		if (_dirIndex == 0)
			return;
		closeFile();
		openFile(_dirFiles.at(--_dirIndex).absoluteFilePath());
	}
	if (event->key() == NEXT_KEY) {
		if (_dirIndex == _dirFiles.size() - 1)
			return;
		closeFile();
		openFile(_dirFiles.at(++_dirIndex).absoluteFilePath());
	}
}

void GUI::setDir(const QString &file)
{
	QDir dir = QFileInfo(file).absoluteDir();
	_dirFiles = dir.entryInfoList(QStringList("*.gpx"), QDir::Files);
	_dirIndex = _dirFiles.empty() ? -1 : _dirFiles.indexOf(file);
}
