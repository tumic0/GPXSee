#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QList>
#include <QHash>
#include "common/kv.h"

class QFile;

typedef KV<QByteArray, QByteArray> HTTPHeader;

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

	const HTTPHeader &header() const {return _header;}
	bool isNull() const {return _header.key().isNull();}

private:
	HTTPHeader _header;
};

class Downloader : public QObject
{
	Q_OBJECT

public:
	Downloader(QObject *parent = 0) : QObject(parent) {}

	bool get(const QList<Download> &list, const QList<HTTPHeader> &headers);
	void clearErrors() {_errorDownloads.clear();}

	static void setNetworkManager(QNetworkAccessManager *manager)
	  {_manager = manager;}
	static void setTimeout(int timeout) {_timeout = timeout;}
	static void enableHTTP2(bool enable);

signals:
	void finished();

private slots:
	void emitFinished();
	void emitReadReady();

private:
	void insertError(const QUrl &url, QNetworkReply::NetworkError error);
	bool doDownload(const Download &dl, const QList<HTTPHeader> &headers);
	void downloadFinished(QNetworkReply *reply);
	void readData(QNetworkReply *reply);

	QHash<QUrl, QFile*> _currentDownloads;
	QHash<QUrl, int> _errorDownloads;

	static QNetworkAccessManager *_manager;
	static int _timeout;
	static bool _http2;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Download &download);
#endif // QT_NO_DEBUG

#endif // DOWNLOADER_H
