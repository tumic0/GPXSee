#include <QtGlobal>
#include <QTranslator>
#include <QLocale>
#include <QFileOpenEvent>
#include <QNetworkProxyFactory>
#include <QLibraryInfo>
#include "map/onlinemap.h"
#include "map/downloader.h"
#include "opengl.h"
#include "gui.h"
#include "app.h"


App::App(int &argc, char **argv) : QApplication(argc, argv),
  _argc(argc), _argv(argv)
{
	QTranslator *gpxsee = new QTranslator(this);
	QString locale = QLocale::system().name();
	gpxsee->load(QString(":/lang/gpxsee_") + locale);
	installTranslator(gpxsee);

	QTranslator *qt = new QTranslator(this);
	qt->load(QLocale::system(), "qt", "_", QLibraryInfo::location(
	  QLibraryInfo::TranslationsPath));
	installTranslator(qt);

#ifdef Q_OS_MAC
	setAttribute(Qt::AA_DontShowIconsInMenus);
#endif // Q_OS_MAC

	QNetworkProxyFactory::setUseSystemConfiguration(true);
	OnlineMap::setDownloader(new Downloader(this));
	OPENGL_SET_SAMPLES(4);

	_gui = new GUI();
}

App::~App()
{
	delete _gui;
}

void App::run()
{
	_gui->show();

	for (int i = 1; i < _argc; i++)
		_gui->openFile(QString::fromLocal8Bit(_argv[i]));

	exec();
}

bool App::event(QEvent *event)
{
	if (event->type() == QEvent::FileOpen) {
		QFileOpenEvent *e = static_cast<QFileOpenEvent *>(event);
		return _gui->openFile(e->file());
	}

	return QApplication::event(event);
}
