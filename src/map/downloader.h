#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QList>
#include <QSet>

class QNetworkReply;

class Download
{
public:
	Download(const QUrl &url, const QString &file) : _url(url), _file(file) {}

	const QUrl& url() const {return _url;}
	const QString& file() const {return _file;}

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

	bool doDownload(const Download &dl, const QByteArray &authorization,
	  const Redirect *redirect = 0);
	bool saveToDisk(const QString &filename, QIODevice *data);

	QSet<QUrl> _currentDownloads;
	QSet<QUrl> _errorDownloads;

	static int _timeout;
	static QNetworkAccessManager *_manager;
};

#endif // DOWNLOADER_H
