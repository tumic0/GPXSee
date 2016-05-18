#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QList>
#include <QSet>


class Download
{
public:
	Download(const QString &url, const QString &file)
		{_url = url; _file = file;}
	const QString& url() const {return _url;}
	const QString& file() const {return _file;}

private:
	QString _url;
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

	bool doDownload(const Download &dl);
	bool saveToDisk(const QString &filename, QIODevice *data);

	QNetworkAccessManager _manager;
	QList<QNetworkReply *> _currentDownloads;
	QSet<QUrl> _errorDownloads;
};

#endif // DOWNLOADER_H
