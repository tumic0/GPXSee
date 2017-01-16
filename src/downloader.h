#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QMap>
#include <QSet>


class Download
{
public:
	Download(const QUrl &url, const QString &file)
		{_url = url; _file = file;}
	const QUrl& url() const {return _url;}
	const QString& file() const {return _file;}

private:
	QUrl _url;
	QString _file;
};


class Downloader : public QObject
{
	Q_OBJECT

public:
	static Downloader& instance()
		{static Downloader i; return i;}
	bool get(const QList<Download> &list);

signals:
	void finished();

private slots:
	void downloadFinished(QNetworkReply *reply);

private:
	Downloader();
	Downloader(Downloader const&);
	void operator=(Downloader const&);

	bool doDownload(const Download &dl, const QUrl &origin = QUrl());
	bool saveToDisk(const QString &filename, QIODevice *data);

	QNetworkAccessManager _manager;
	QMap<QUrl, QNetworkReply *> _currentDownloads;
	QSet<QUrl> _errorDownloads;
};

#endif // DOWNLOADER_H
