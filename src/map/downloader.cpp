#include <QFile>
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
#define TMP_SUFFIX ".download"

// QNetworkReply::errorString() returns bullshit, use our own reporting
static const char *errorString(QNetworkReply::NetworkError error)
{
	switch (error) {
		case QNetworkReply::ConnectionRefusedError:
			return "Connection refused";
		case QNetworkReply::RemoteHostClosedError:
			return "Connection closed";
		case QNetworkReply::HostNotFoundError:
			return "Host not found";
		case QNetworkReply::TimeoutError:
			return "Connection timeout";
		case QNetworkReply::OperationCanceledError:
			return "Operation canceled";
		case QNetworkReply::SslHandshakeFailedError:
			return "SSL handshake failed";
		case QNetworkReply::TemporaryNetworkFailureError:
			return "Temporary network failure";
		case QNetworkReply::NetworkSessionFailedError:
			return "Network session failed";
		case QNetworkReply::BackgroundRequestNotAllowedError:
			return "Background request not allowed";
		case QNetworkReply::TooManyRedirectsError:
			return "Too many redirects";
		case QNetworkReply::InsecureRedirectError:
			return "Insecure redirect";
		case QNetworkReply::ProxyConnectionRefusedError:
			return "Proxy connection refused";
		case QNetworkReply::ProxyConnectionClosedError:
			return "Proxy connection closed";
		case QNetworkReply::ProxyNotFoundError:
			return "Proxy not found";
		case QNetworkReply::ProxyTimeoutError:
			return "Proxy timeout error";
		case QNetworkReply::ProxyAuthenticationRequiredError:
			return "Proxy authentication required";
		case QNetworkReply::ContentAccessDenied:
			return "Content access denied";
		case QNetworkReply::ContentOperationNotPermittedError:
			return "Content operation not permitted";
		case QNetworkReply::ContentNotFoundError:
			return "Content not found";
		case QNetworkReply::AuthenticationRequiredError:
			return "Authentication required";
		case QNetworkReply::ContentReSendError:
			return "Content re-send error";
		case QNetworkReply::ContentConflictError:
			return "Content conflict";
		case QNetworkReply::ContentGoneError:
			return "Content gone";
		case QNetworkReply::InternalServerError:
			return "Internal server error";
		case QNetworkReply::OperationNotImplementedError:
			return "Operation not implemented";
		case QNetworkReply::ServiceUnavailableError:
			return "Service unavailable";
		case QNetworkReply::ProtocolUnknownError:
			return "Protocol unknown";
		case QNetworkReply::ProtocolInvalidOperationError:
			return "Protocol invalid operation";
		case QNetworkReply::UnknownNetworkError:
			return "Unknown network error";
		case QNetworkReply::UnknownProxyError:
			return "Unknown proxy error";
		case QNetworkReply::UnknownContentError:
			return "Unknown content error";
		case QNetworkReply::ProtocolFailure:
			return "Protocol failure";
		case QNetworkReply::UnknownServerError:
			return "Unknown server error";
		default:
			return "Unknown error";
	}
}

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

QNetworkAccessManager *Downloader::_manager = 0;
int Downloader::_timeout = 30;
bool Downloader::_http2 = true;

bool Downloader::doDownload(const Download &dl, const QList<HTTPHeader> &headers)
{
	const QUrl &url = dl.url();
	bool userAgent = false;

	if (!url.isValid() || !(url.scheme() == QLatin1String("http")
	  || url.scheme() == QLatin1String("https"))) {
		qWarning("%s: Invalid URL", qUtf8Printable(url.toString()));
		return false;
	}

	if (_errorDownloads.value(url) >= RETRIES)
		return false;
	if (_currentDownloads.contains(url))
		return false;

	QNetworkRequest request(url);
	request.setMaximumRedirectsAllowed(MAX_REDIRECT_LEVEL);
	request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
	  QNetworkRequest::NoLessSafeRedirectPolicy);
	request.setAttribute(QNetworkRequest::Http2AllowedAttribute, QVariant(_http2));
	request.setTransferTimeout(_timeout * 1000);

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
		qWarning("%s: %s", qUtf8Printable(file->fileName()),
		  qUtf8Printable(file->errorString()));
		delete file;
		_errorDownloads.insert(url, RETRIES);
		return false;
	}

	Q_ASSERT(_manager);
	QNetworkReply *reply = _manager->get(request);
	file->setParent(reply);
	_currentDownloads.insert(url, file);

	if (reply->isRunning()) {
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
	switch (error) {
		case QNetworkReply::OperationCanceledError:
		case QNetworkReply::TimeoutError:
		case QNetworkReply::RemoteHostClosedError:
		case QNetworkReply::ConnectionRefusedError:
			_errorDownloads.insert(url, _errorDownloads.value(url) + 1);
			break;
		default:
			_errorDownloads.insert(url, RETRIES);
	}
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
		qWarning("%s: %s", url.toEncoded().constData(), errorString(error));
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
