#include <QtGlobal>
#include <QTranslator>
#include <QLocale>
#include <QFileOpenEvent>
#include <QNetworkProxyFactory>
#include <QNetworkAccessManager>
#include <QLibraryInfo>
#include <QSettings>
#include "common/programpaths.h"
#include "common/config.h"
#include "map/downloader.h"
#include "map/ellipsoid.h"
#include "map/gcs.h"
#include "map/pcs.h"
#include "data/dem.h"
#include "opengl.h"
#include "gui.h"
#include "settings.h"
#include "app.h"


App::App(int &argc, char **argv) : QApplication(argc, argv)
{
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
	setApplicationName(APP_NAME);
#else
	setApplicationName(QString(APP_NAME).toLower());
#endif
	setApplicationVersion(APP_VERSION);

	QTranslator *gpxsee = new QTranslator(this);
	gpxsee->load(QLocale::system(), "gpxsee", "_",
	  ProgramPaths::translationsDir());
	installTranslator(gpxsee);

	QTranslator *qt = new QTranslator(this);
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
	qt->load(QLocale::system(), "qt", "_", ProgramPaths::translationsDir());
#else // Q_OS_WIN32 || Q_OS_MAC
	qt->load(QLocale::system(), "qt", "_", QLibraryInfo::location(
	  QLibraryInfo::TranslationsPath));
#endif // Q_OS_WIN32 || Q_OS_MAC
	installTranslator(qt);

#ifdef Q_OS_MAC
	setAttribute(Qt::AA_DontShowIconsInMenus);
#endif // Q_OS_MAC
	QNetworkProxyFactory::setUseSystemConfiguration(true);
	/* The QNetworkAccessManager must be a child of QApplication, otherwise it
	   triggers the following warning on exit (and may probably crash):
	   "QThreadStorage: Thread X exited after QThreadStorage Y destroyed" */
	Downloader::setNetworkManager(new QNetworkAccessManager(this));
	DEM::setDir(ProgramPaths::demDir());
	OPENGL_SET_FORMAT(4, 8);

	loadDatums();
	loadPCSs();

	QSettings settings(qApp->applicationName(), qApp->applicationName());
	settings.beginGroup(OPTIONS_SETTINGS_GROUP);
#ifdef ENABLE_HTTP2
	Downloader::enableHTTP2(settings.value(ENABLE_HTTP2_SETTING,
	  ENABLE_HTTP2_DEFAULT).toBool());
#endif // ENABLE_HTTP2
	Downloader::setTimeout(settings.value(CONNECTION_TIMEOUT_SETTING,
	  CONNECTION_TIMEOUT_DEFAULT).toInt());
	settings.endGroup();

	_gui = new GUI();
}

App::~App()
{
	delete _gui;
}

int App::run()
{
	_gui->show();

	QStringList args(arguments());
	for (int i = 1; i < args.count(); i++)
		_gui->openFile(args.at(i));

	return exec();
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
	QString ellipsoidsFile(ProgramPaths::ellipsoidsFile());
	QString gcsFile(ProgramPaths::gcsFile());

	if (ellipsoidsFile.isNull())
		qWarning("No ellipsoids file found.");
	if (gcsFile.isNull())
		qWarning("No GCS file found.");

	if (!ellipsoidsFile.isNull() && !gcsFile.isNull()) {
		Ellipsoid::loadList(ellipsoidsFile);
		GCS::loadList(gcsFile);
	} else
		qWarning("Maps based on a datum different from WGS84 won't work.");
}

void App::loadPCSs()
{
	QString pcsFile(ProgramPaths::pcsFile());

	if (pcsFile.isNull())
		qWarning("No PCS file found.");
	else
		PCS::loadList(pcsFile);
}
