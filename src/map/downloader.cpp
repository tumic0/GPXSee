#include <QFile>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "config.h"
#include "downloader.h"


#if defined(Q_OS_LINUX)
#define PLATFORM_STR "Linux"
#elif defined(Q_OS_WIN32)
#define PLATFORM_STR "Windows"
#elif defined(Q_OS_MAC)
#define PLATFORM_STR "OS X"
#else
#define PLATFORM_STR "Unknown"
#endif

#define USER_AGENT \
	APP_NAME "/" APP_VERSION " (" PLATFORM_STR "; Qt " QT_VERSION_STR ")"

#define ATTR_REDIRECT QNetworkRequest::RedirectionTargetAttribute
#define ATTR_FILE     QNetworkRequest::User
#define ATTR_ORIGIN   (QNetworkRequest::Attribute)(QNetworkRequest::User + 1)
#define ATTR_LEVEL    (QNetworkRequest::Attribute)(QNetworkRequest::User + 2)

#define MAX_REDIRECT_LEVEL 5


Downloader::Downloader(QObject *parent) : QObject(parent)
{
	connect(&_manager, SIGNAL(finished(QNetworkReply*)),
			SLOT(downloadFinished(QNetworkReply*)));
}

bool Downloader::doDownload(const Download &dl, const Redirect &redirect)
{
	QUrl url(dl.url());

	if (_errorDownloads.contains(url))
		return false;
	if (_currentDownloads.contains(url))
		return false;

	QNetworkRequest request(url);
	request.setAttribute(ATTR_FILE, QVariant(dl.file()));
	if (!redirect.isNull()) {
		request.setAttribute(ATTR_ORIGIN, QVariant(redirect.origin()));
		request.setAttribute(ATTR_LEVEL, QVariant(redirect.level()));
	}
	request.setRawHeader("User-Agent", USER_AGENT);
	QNetworkReply *reply = _manager.get(request);

	_currentDownloads.insert(url, reply);

	return true;
}

bool Downloader::saveToDisk(const QString &filename, QIODevice *data)
{
	QFile file(filename);

	if (!file.open(QIODevice::WriteOnly)) {
		qWarning("Error writing map tile: %s: %s\n",
		  qPrintable(filename), qPrintable(file.errorString()));
		return false;
	}

	file.write(data->readAll());
	file.close();

	return true;
}

void Downloader::downloadFinished(QNetworkReply *reply)
{
	QUrl url = reply->request().url();

	if (reply->error()) {
		QUrl origin = reply->request().attribute(ATTR_ORIGIN).toUrl();
		if (origin.isEmpty()) {
			_errorDownloads.insert(url);
			qWarning("Error downloading map tile: %s: %s\n",
			  url.toEncoded().constData(), qPrintable(reply->errorString()));
		} else {
			_errorDownloads.insert(origin);
			qWarning("Error downloading map tile: %s -> %s: %s\n",
			  origin.toEncoded().constData(), url.toEncoded().constData(),
			  qPrintable(reply->errorString()));
		}
	} else {
		QUrl location = reply->attribute(ATTR_REDIRECT).toUrl();
		QString filename = reply->request().attribute(ATTR_FILE)
		  .toString();

		if (!location.isEmpty()) {
			QUrl origin = reply->request().attribute(ATTR_ORIGIN).toUrl();
			int level = reply->request().attribute(ATTR_LEVEL).toInt();

			if (location == url) {
				_errorDownloads.insert(url);
				qWarning("Error downloading map tile: %s: "
				  "redirect loop\n", url.toEncoded().constData());
			} else if (level >= MAX_REDIRECT_LEVEL) {
				_errorDownloads.insert(origin);
				qWarning("Error downloading map tile: %s: "
				  "redirect level limit reached\n",
				  origin.toEncoded().constData());
			} else {
				Redirect redirect(origin.isEmpty() ? url : origin, level + 1);
				Download dl(location, filename);
				doDownload(dl, redirect);
			}
		} else
			if (!saveToDisk(filename, reply))
				_errorDownloads.insert(url);
	}

	_currentDownloads.remove(url);
	reply->deleteLater();

	if (_currentDownloads.isEmpty())
		emit finished();
}

bool Downloader::get(const QList<Download> &list)
{
	bool finishEmitted = false;

	for (int i = 0; i < list.count(); i++)
		finishEmitted |= doDownload(list.at(i));

	return finishEmitted;
}
