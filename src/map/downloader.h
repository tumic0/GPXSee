#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QList>
#include <QSet>
#include <QHash>
#include "config.h"


class Download
{
public:
	Download(const QUrl &url, const QString &file) : _url(url), _file(file) {}

	const QUrl &url() const {return _url;}
	const QString &file() const {return _file;}

private:
	QUrl _url;
	QString _file;
};

class Authorization
{
public:
	Authorization() {}
	Authorization(const QString &username, const QString &password);

	const QByteArray &header() const {return _header;}

private:
	QByteArray _header;
};

class Downloader : public QObject
{
	Q_OBJECT

public:
	Downloader(QObject *parent = 0) : QObject(parent) {}

	bool get(const QList<Download> &list, const Authorization &authorization
	  = Authorization());
	void clearErrors() {_errorDownloads.clear();}

	static void setTimeout(int timeout) {_timeout = timeout;}
#ifdef ENABLE_HTTP2
	static void enableHTTP2(bool enable);
#endif // ENABLE_HTTP2
	static void setNetworkAccessManager(QNetworkAccessManager *manager)
	  {_manager = manager;}

signals:
	void finished();

private slots:
	void emitFinished();
	void downloadFinished(QNetworkReply *reply);

private:
	class Redirect;
	class ReplyTimeout;

	void insertError(const QUrl &url, QNetworkReply::NetworkError error);
	bool doDownload(const Download &dl, const QByteArray &authorization,
	  const Redirect *redirect = 0);
	bool saveToDisk(const QString &filename, QIODevice *data);

	QSet<QUrl> _currentDownloads;
	QHash<QUrl, int> _errorDownloads;

	static int _timeout;
#ifdef ENABLE_HTTP2
	static bool _http2;
#endif // ENABLE_HTTP2
	static QNetworkAccessManager *_manager;
};

#endif // DOWNLOADER_H
