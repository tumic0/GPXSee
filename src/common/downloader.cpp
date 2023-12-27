#include <QFile>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QDir>
#include <QTimerEvent>
#include "common/config.h"
#include "downloader.h"


#if defined(Q_OS_ANDROID)
#define PLATFORM_STR "Android"
#elif defined(Q_OS_LINUX)
#define PLATFORM_STR "Linux"
#elif defined(Q_OS_WIN32)
#define PLATFORM_STR "Windows"
#elif defined(Q_OS_MAC)
#define PLATFORM_STR "OS X"
#elif defined(Q_OS_BSD4)
#define PLATFORM_STR "BSD"
#elif defined(Q_OS_HAIKU)
#define PLATFORM_STR "Haiku"
#else
#define PLATFORM_STR "Unknown"
#endif

#define USER_AGENT \
	APP_NAME "/" APP_VERSION " (" PLATFORM_STR "; Qt " QT_VERSION_STR ")"

#define MAX_REDIRECT_LEVEL 5
#define RETRIES 3

#define ATTR_REDIRECT_POLICY QNetworkRequest::RedirectPolicyAttribute
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#define ATTR_HTTP2_ALLOWED QNetworkRequest::HTTP2AllowedAttribute
#else // QT 5.15
#define ATTR_HTTP2_ALLOWED QNetworkRequest::Http2AllowedAttribute
#endif // QT 5.15

#define TMP_SUFFIX ".download"


static QString tmpName(const QString &origName)
{
	return origName + TMP_SUFFIX;
}

static QString origName(const QString &tmpName)
{
	return tmpName.left(tmpName.size() - (sizeof(TMP_SUFFIX) - 1));
}

Authorization::Authorization(const QString &username, const QString &password)
{
	QString concatenated = username + ":" + password;
	QByteArray data = concatenated.toLocal8Bit().toBase64();
	_header = HTTPHeader("Authorization", "Basic " + data);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
NetworkTimeout::NetworkTimeout(int timeout, QNetworkReply *reply)
  : QObject(reply), _timeout(timeout)
{
	connect(reply, &QIODevice::readyRead, this, &NetworkTimeout::reset);
	_timer.start(timeout * 1000, this);
}

void NetworkTimeout::reset()
{
	_timer.start(_timeout * 1000, this);
}

void NetworkTimeout::timerEvent(QTimerEvent *ev)
{
	if (!_timer.isActive() || ev->timerId() != _timer.timerId())
		return;
	QNetworkReply *reply = static_cast<QNetworkReply*>(parent());
	if (reply->isRunning())
		reply->abort();
	_timer.stop();
}
#endif // QT 5.15


QNetworkAccessManager *Downloader::_manager = 0;
int Downloader::_timeout = 30;
bool Downloader::_http2 = true;

bool Downloader::doDownload(const Download &dl, const QList<HTTPHeader> &headers)
{
	const QUrl &url = dl.url();
	bool userAgent = false;

	if (!url.isValid() || !(url.scheme() == QLatin1String("http")
	  || url.scheme() == QLatin1String("https"))) {
		qWarning("%s: Invalid URL", qPrintable(url.toString()));
		return false;
	}

	if (_errorDownloads.value(url) >= RETRIES)
		return false;
	if (_currentDownloads.contains(url))
		return false;

	QNetworkRequest request(url);
	request.setMaximumRedirectsAllowed(MAX_REDIRECT_LEVEL);
	request.setAttribute(ATTR_REDIRECT_POLICY,
	  QNetworkRequest::NoLessSafeRedirectPolicy);
	request.setAttribute(ATTR_HTTP2_ALLOWED, QVariant(_http2));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	request.setTransferTimeout(_timeout * 1000);
#endif // QT 5.15

	for (int i = 0; i < headers.size(); i++) {
		const HTTPHeader &hdr = headers.at(i);
		request.setRawHeader(hdr.key(), hdr.value());
		// QByteArray::compare() not available in Qt < 5.12
		if (!QString(hdr.key()).compare("User-Agent", Qt::CaseInsensitive))
			userAgent = true;
	}
	if (!userAgent)
		request.setRawHeader("User-Agent", USER_AGENT);

	QFile *file = new QFile(tmpName(dl.file()));
	if (!file->open(QIODevice::WriteOnly)) {
		qWarning("%s: %s", qPrintable(file->fileName()),
		  qPrintable(file->errorString()));
		_errorDownloads.insert(url, RETRIES);
		return false;
	}

	Q_ASSERT(_manager);
	QNetworkReply *reply = _manager->get(request);
	file->setParent(reply);
	_currentDownloads.insert(url, file);

	if (reply->isRunning()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
		new NetworkTimeout(_timeout, reply);
#endif // QT 5.15
		connect(reply, &QIODevice::readyRead, this, &Downloader::emitReadReady);
		connect(reply, &QNetworkReply::finished, this, &Downloader::emitFinished);
	} else {
		readData(reply);
		downloadFinished(reply);
	}

	return true;
}

void Downloader::emitFinished()
{
	downloadFinished(static_cast<QNetworkReply*>(sender()));
}

void Downloader::emitReadReady()
{
	readData(static_cast<QNetworkReply*>(sender()));
}

void Downloader::insertError(const QUrl &url, QNetworkReply::NetworkError error)
{
	if (error == QNetworkReply::OperationCanceledError)
		_errorDownloads.insert(url, _errorDownloads.value(url) + 1);
	else
		_errorDownloads.insert(url, RETRIES);
}

void Downloader::readData(QNetworkReply *reply)
{
	QFile *file = _currentDownloads.value(reply->request().url());
	Q_ASSERT(file);
	file->write(reply->readAll());
}

void Downloader::downloadFinished(QNetworkReply *reply)
{
	QUrl url(reply->request().url());
	QNetworkReply::NetworkError error = reply->error();

	QFile *file = _currentDownloads.value(reply->request().url());
	if (error) {
		insertError(url, error);
		qWarning("%s: %s", url.toEncoded().constData(),
		  qPrintable(reply->errorString()));
		file->remove();
	} else {
		file->close();
		file->rename(origName(file->fileName()));
	}

	_currentDownloads.remove(url);
	reply->deleteLater();

	if (_currentDownloads.isEmpty())
		emit finished();
}

bool Downloader::get(const QList<Download> &list,
  const QList<HTTPHeader> &headers)
{
	bool finishEmitted = false;

	for (int i = 0; i < list.count(); i++)
		finishEmitted |= doDownload(list.at(i), headers);

	return finishEmitted;
}

void Downloader::enableHTTP2(bool enable)
{
	Q_ASSERT(_manager);
	_http2 = enable;
	_manager->clearConnectionCache();
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Download &download)
{
	dbg.nospace() << "Download(" << download.url() << "," << download.file()
	  << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
