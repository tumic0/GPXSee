#include <QtGlobal>
#include <QTranslator>
#include <QLocale>
#include <QFileOpenEvent>
#include <QNetworkProxyFactory>
#include <QNetworkAccessManager>
#include <QLibraryInfo>
#include <QSettings>
#include "map/downloader.h"
#include "map/ellipsoid.h"
#include "map/gcs.h"
#include "map/pcs.h"
#include "opengl.h"
#include "gui.h"
#include "config.h"
#include "settings.h"
#include "app.h"


App::App(int &argc, char **argv) : QApplication(argc, argv),
  _argc(argc), _argv(argv)
{
	QTranslator *gpxsee = new QTranslator(this);
	gpxsee->load(QLocale::system(), "gpxsee", "_", TRANSLATIONS_DIR);
	installTranslator(gpxsee);

	QTranslator *qt = new QTranslator(this);
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
	qt->load(QLocale::system(), "qt", "_", TRANSLATIONS_DIR);
#else // Q_OS_WIN32 || Q_OS_MAC
	qt->load(QLocale::system(), "qt", "_", QLibraryInfo::location(
	  QLibraryInfo::TranslationsPath));
#endif // Q_OS_WIN32 || Q_OS_MAC
	installTranslator(qt);

#ifdef Q_OS_MAC
	setAttribute(Qt::AA_DontShowIconsInMenus);
#endif // Q_OS_MAC

	QNetworkProxyFactory::setUseSystemConfiguration(true);
	QSettings settings(APP_NAME, APP_NAME);
	settings.beginGroup(OPTIONS_SETTINGS_GROUP);
#ifdef ENABLE_HTTP2
	Downloader::enableHTTP2(settings.value(ENABLE_HTTP2_SETTING,
	  ENABLE_HTTP2_DEFAULT).toBool());
#endif // ENABLE_HTTP2
	Downloader::setTimeout(settings.value(CONNECTION_TIMEOUT_SETTING,
	  CONNECTION_TIMEOUT_DEFAULT).toInt());
	settings.endGroup();

	OPENGL_SET_SAMPLES(4);
	loadDatums();
	loadPCSs();

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

	if (QFile::exists(USER_ELLIPSOID_FILE))
		ef = USER_ELLIPSOID_FILE;
	else if (QFile::exists(GLOBAL_ELLIPSOID_FILE))
		ef = GLOBAL_ELLIPSOID_FILE;
	else
		qWarning("No ellipsoids file found.");

	if (QFile::exists(USER_GCS_FILE))
		df = USER_GCS_FILE;
	else if (QFile::exists(GLOBAL_GCS_FILE))
		df = GLOBAL_GCS_FILE;
	else
		qWarning("No datums file found.");

	if (!ef.isNull() && !df.isNull()) {
		Ellipsoid::loadList(ef);
		GCS::loadList(df);
	} else
		qWarning("Maps based on a datum different from WGS84 won't work.");
}

void App::loadPCSs()
{
	QString file;

	if (QFile::exists(USER_PCS_FILE))
		file = USER_PCS_FILE;
	else if (QFile::exists(GLOBAL_PCS_FILE))
		file = GLOBAL_PCS_FILE;
	else {
		qWarning("No PCS file found.");
		return;
	}

	PCS::loadList(file);
}
