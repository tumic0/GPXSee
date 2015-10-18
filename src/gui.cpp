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
#include "config.h"
#include "icons.h"
#include "keys.h"
#include "gpx.h"
#include "elevationgraph.h"
#include "speedgraph.h"
#include "track.h"
#include "infoitem.h"
#include "gui.h"

#include <QDebug>



static QString timeSpan(qreal time)
{
	unsigned h, m, s;

	h = time / 3600;
	m = (time - (h * 3600)) / 60;
	s = time - (h * 3600) - (m * 60);

	return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0'))
	  .arg(s,2, 10, QChar('0'));
}

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
	_files = 0;
	_distance = 0;
	_time = 0;

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
#ifndef __APPLE__
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);
#endif // __APPLE__

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
	_elevationGraph = new ElevationGraph;
	_speedGraph = new SpeedGraph;

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
	QString lastFile;

	for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
		if (openFile(*it))
			lastFile = *it;

	if (!lastFile.isEmpty())
		setDir(lastFile);
}

bool GUI::openFile(const QString &fileName)
{
	GPX gpx;

	if (!fileName.isEmpty()) {
		if (gpx.loadFile(fileName)) {
			_elevationGraph->loadGPX(gpx);
			_speedGraph->loadGPX(gpx);
			_track->loadGPX(gpx);
			if (_showPOIAction->isChecked())
				_track->loadPOI(_poi);

			_distance += gpx.distance();
			_time += gpx.time();

			updateStatusBarInfo(fileName);

			_fileActionGroup->setEnabled(true);

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

	_track->plot(&p, QRectF(0, 300, printer.width(), (0.80 * printer.height())
	  - 400));
	_elevationGraph->plot(&p,  QRectF(0, 0.80 * printer.height(),
	  printer.width(), printer.height() * 0.20));

	QGraphicsScene scene;
	InfoItem info;
	info.insert(tr("Distance"), QString::number(_distance / 1000, 'f', 1) + " "
	  + tr("km"));
	info.insert(tr("Time"), timeSpan(_time));
	info.insert(tr("Ascent"), QString::number(_elevationGraph->ascent(), 'f', 0)
	  + " " + tr("m"));
	info.insert(tr("Descent"), QString::number(_elevationGraph->descent(), 'f',
	  0) + " " + tr("m"));
	info.insert(tr("Maximum"), QString::number(_elevationGraph->max(), 'f', 0)
	  + " " + tr("m"));
	info.insert(tr("Minimum"), QString::number(_elevationGraph->min(), 'f', 0)
	  + " " + tr("m"));
	scene.addItem(&info);
	scene.render(&p, QRectF(0, 0, printer.width(), 200));

	p.end();
}

void GUI::closeFile()
{
	_files = 0;
	_distance = 0;
	_time = 0;

	_elevationGraph->clear();
	_speedGraph->clear();
	_track->clear();
	_fileNameLabel->clear();
	_distanceLabel->clear();
	_timeLabel->clear();

	_fileActionGroup->setEnabled(false);
}

void GUI::showPOI()
{
	if (_showPOIAction->isChecked())
		_track->loadPOI(_poi);
	else
		_track->clearPOI();
}

void GUI::updateStatusBarInfo(const QString &fileName)
{
	if (++_files > 1)
		_fileNameLabel->setText(tr("%1 tracks").arg(_files));
	else
		_fileNameLabel->setText(fileName);

	_distanceLabel->setText(QString::number(_distance / 1000, 'f', 1)
	  + " " + tr("km"));
	_timeLabel->setText(timeSpan(_time));
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
