#include <QtGlobal>
#include <QTranslator>
#include <QLocale>
#include <QFileOpenEvent>
#include <QNetworkProxyFactory>
#include <QLibraryInfo>
#include "map/onlinemap.h"
#include "map/downloader.h"
#include "map/ellipsoid.h"
#include "map/datum.h"
#include "opengl.h"
#include "gui.h"
#include "config.h"
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
	loadDatums();

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

void App::loadDatums()
{
	QString ef, df;
	bool ok = false;

	if (QFile::exists(USER_ELLIPSOID_FILE))
		ef = USER_ELLIPSOID_FILE;
	else if (QFile::exists(GLOBAL_ELLIPSOID_FILE))
		ef = GLOBAL_ELLIPSOID_FILE;
	else
		qWarning("No ellipsoids file found.");

	if (QFile::exists(USER_DATUM_FILE))
		df = USER_DATUM_FILE;
	else if (QFile::exists(GLOBAL_DATUM_FILE))
		df = GLOBAL_DATUM_FILE;
	else
		qWarning("No datums file found.");

	if (!ef.isNull() && !df.isNull()) {
		if (!Ellipsoid::loadList(ef)) {
			if (Ellipsoid::errorLine())
				qWarning("%s: parse error on line %d: %s", qPrintable(ef),
				  Ellipsoid::errorLine(), qPrintable(Ellipsoid::errorString()));
			else
				qWarning("%s: %s", qPrintable(ef), qPrintable(
				  Ellipsoid::errorString()));
		} else {
			if (!Datum::loadList(df)) {
				if (Datum::errorLine())
					qWarning("%s: parse error on line %d: %s", qPrintable(df),
					  Datum::errorLine(), qPrintable(Datum::errorString()));
				else
					qWarning("%s: %s", qPrintable(ef), qPrintable(
					  Datum::errorString()));
			} else
				ok = true;
		}
	}

	if (!ok)
		qWarning("Maps based on a datum different from WGS84 won't work.");
}
