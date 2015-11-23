#include <QFile>
#include <QFileInfo>
#include "downloader.h"

#include <QDebug>


Downloader::Downloader()
{
	connect(&manager, SIGNAL(finished(QNetworkReply*)),
			SLOT(downloadFinished(QNetworkReply*)));
}

void Downloader::doDownload(const Download &dl)
{
	QUrl url(dl.url());
	QNetworkRequest request(url);
	request.setAttribute(QNetworkRequest::User, QVariant(dl.file()));
	QNetworkReply *reply = manager.get(request);

	currentDownloads.append(reply);
}

bool Downloader::saveToDisk(const QString &filename, QIODevice *data)
{
	QFile file(filename);

	if (!file.open(QIODevice::WriteOnly)) {
		fprintf(stderr, "Could not open %s for writing: %s\n",
		  qPrintable(filename), qPrintable(file.errorString()));
		return false;
	}

	file.write(data->readAll());
	file.close();

	return true;
}

void Downloader::downloadFinished(QNetworkReply *reply)
{
	QUrl url = reply->url();
	if (reply->error()) {
		fprintf(stderr, "Download of %s failed: %s\n",
		  url.toEncoded().constData(), qPrintable(reply->errorString()));
	} else {
		QString filename = reply->request().attribute(QNetworkRequest::User)
		  .toString();
		saveToDisk(filename, reply);
	}

	currentDownloads.removeAll(reply);
	reply->deleteLater();

	if (currentDownloads.isEmpty())
		emit finished();
}

void Downloader::get(const QList<Download> &list)
{
	for (int i = 0; i < list.count(); i++)
		doDownload(list.at(i));
}
