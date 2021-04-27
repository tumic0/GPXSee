#include <QFile>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QBasicTimer>
#include <QDir>
#include <QTimerEvent>
#include "common/config.h"
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
#define ATTR_ORIGIN \
  static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 1)
#define ATTR_LEVEL \
  static_cast<QNetworkRequest::Attribute>(QNetworkRequest::User + 2)

#define MAX_REDIRECT_LEVEL 5
#define RETRIES 3


Authorization::Authorization(const QString &username, const QString &password)
{
	QString concatenated = username + ":" + password;
	QByteArray data = concatenated.toLocal8Bit().toBase64();
	_header = "Basic " + data;
}

class Downloader::ReplyTimeout : public QObject
{
public:
	static void setTimeout(QNetworkReply *reply, int timeout)
	{
		Q_ASSERT(reply);
		new ReplyTimeout(reply, timeout);
	}

private:
	ReplyTimeout(QNetworkReply *reply, int timeout) : QObject(reply)
	{
		_timer.start(timeout * 1000, this);
	}

	void timerEvent(QTimerEvent *ev)
	{
		if (!_timer.isActive() || ev->timerId() != _timer.timerId())
			return;
		QNetworkReply *reply = static_cast<QNetworkReply*>(parent());
		if (reply->isRunning())
			reply->close();
		_timer.stop();
	}

	QBasicTimer _timer;
};

class Downloader::Redirect
{
public:
	Redirect() : _level(0) {}
	Redirect(const QUrl &origin, int level) :
	  _origin(origin), _level(level) {}

	const QUrl &origin() const {return _origin;}
	int level() const {return _level;}

private:
	QUrl _origin;
	int _level;
};

QNetworkAccessManager *Downloader::_manager = 0;
int Downloader::_timeout = 30;
bool Downloader::_http2 = true;

bool Downloader::doDownload(const Download &dl,
  const QByteArray &authorization, const Redirect *redirect)
{
	const QUrl &url = dl.url();

	if (!url.isValid() || !(url.scheme() == QLatin1String("http")
	  || url.scheme() == QLatin1String("https"))) {
		qWarning("%s: Invalid URL", qPrintable(url.toString()));
		if (redirect)
			_errorDownloads.insert(redirect->origin(), RETRIES);
		return false;
	}

	if (_errorDownloads.value(url) >= RETRIES)
		return false;
	if (_currentDownloads.contains(url) && !redirect)
		return false;

	QNetworkRequest request(url);
	request.setAttribute(ATTR_FILE, QVariant(dl.file()));
	if (redirect) {
		request.setAttribute(ATTR_ORIGIN, QVariant(redirect->origin()));
		request.setAttribute(ATTR_LEVEL, QVariant(redirect->level()));
	}
	request.setRawHeader("User-Agent", USER_AGENT);
	if (!authorization.isNull())
		request.setRawHeader("Authorization", authorization);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	request.setAttribute(QNetworkRequest::HTTP2AllowedAttribute,
	  QVariant(_http2));
#else // QT 5.15
	request.setAttribute(QNetworkRequest::Http2AllowedAttribute,
	  QVariant(_http2));
#endif // QT 5.15

	Q_ASSERT(_manager);
	QNetworkReply *reply = _manager->get(request);
	if (reply && reply->isRunning()) {
		_currentDownloads.insert(url);
		ReplyTimeout::setTimeout(reply, _timeout);
		connect(reply, &QNetworkReply::finished, this, &Downloader::emitFinished);
	} else if (reply)
		downloadFinished(reply);
	else
		return false;

	return true;
}

void Downloader::emitFinished()
{
	downloadFinished(static_cast<QNetworkReply*>(sender()));
}

bool Downloader::saveToDisk(const QString &filename, QIODevice *data)
{
	QFile file(filename);

	if (!file.open(QIODevice::WriteOnly)) {
		qWarning("Error writing file: %s: %s",
		  qPrintable(filename), qPrintable(file.errorString()));
		return false;
	}

	file.write(data->readAll());
	file.close();

	return true;
}

void Downloader::insertError(const QUrl &url, QNetworkReply::NetworkError error)
{
	if (error == QNetworkReply::OperationCanceledError)
		_errorDownloads.insert(url, _errorDownloads.value(url) + 1);
	else
		_errorDownloads.insert(url, RETRIES);
}

void Downloader::downloadFinished(QNetworkReply *reply)
{
	QUrl url(reply->request().url());
	QNetworkReply::NetworkError error = reply->error();

	if (error) {
		QUrl origin(reply->request().attribute(ATTR_ORIGIN).toUrl());
		if (origin.isEmpty()) {
			insertError(url, error);
			qWarning("Error downloading file: %s: %s",
			  url.toEncoded().constData(), qPrintable(reply->errorString()));
		} else {
			insertError(origin, error);
			qWarning("Error downloading file: %s -> %s: %s",
			  origin.toEncoded().constData(), url.toEncoded().constData(),
			  qPrintable(reply->errorString()));
		}
	} else {
		QUrl location(reply->attribute(ATTR_REDIRECT).toUrl());
		QString filename(reply->request().attribute(ATTR_FILE).toString());

		if (!location.isEmpty()) {
			QUrl origin(reply->request().attribute(ATTR_ORIGIN).toUrl());
			int level = reply->request().attribute(ATTR_LEVEL).toInt();

			if (level >= MAX_REDIRECT_LEVEL) {
				_errorDownloads.insert(origin, RETRIES);
				qWarning("Error downloading file: %s: "
				  "redirect level limit reached (redirect loop?)",
				  origin.toEncoded().constData());
			} else {
				QUrl redirectUrl;
				if (location.isRelative()) {
					QString path = QDir::isAbsolutePath(location.path())
					  ? location.path() : "/" + location.path();
					redirectUrl = QUrl(url.scheme() + "://" + url.host() + path);
				} else
					redirectUrl = location;

				Redirect redirect(origin.isEmpty() ? url : origin, level + 1);
				Download dl(redirectUrl, filename);
				doDownload(dl, reply->request().rawHeader("Authorization"),
				  &redirect);
			}
		} else {
			if (!saveToDisk(filename, reply))
				_errorDownloads.insert(url, RETRIES);
		}
	}

	_currentDownloads.remove(url);
	reply->deleteLater();

	if (_currentDownloads.isEmpty())
		emit finished();
}

bool Downloader::get(const QList<Download> &list,
  const Authorization &authorization)
{
	bool finishEmitted = false;

	for (int i = 0; i < list.count(); i++)
		finishEmitted |= doDownload(list.at(i), authorization.header());

	return finishEmitted;
}

void Downloader::enableHTTP2(bool enable)
{
	Q_ASSERT(_manager);
	_http2 = enable;
	_manager->clearConnectionCache();
}
