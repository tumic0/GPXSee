#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QObject>
#include <QFileInfo>
#include <QStringList>

class QFileSystemWatcher;

class FileBrowser : public QObject
{
	Q_OBJECT

public:
	FileBrowser(QObject *parent = 0);

#ifdef Q_OS_ANDROID
	void setCurrentDir(const QString &path);
#else // Q_OS_ANDROID
	void setCurrent(const QString &path);
#endif // Q_OS_ANDROID
	void setFilter(const QStringList &filter);

	QString current();
	QString next();
	QString prev();
	QString last();
	QString first();

	bool isLast() const;
	bool isFirst() const;

signals:
	void listChanged();

private slots:
	void reloadDirectory(const QString &path);

private:
#ifndef Q_OS_ANDROID
	QFileSystemWatcher *_watcher;
#endif // Q_OS_ANDROID
	QStringList _filter;
	QFileInfoList _files;
	int _index;
};

#endif // FILEBROWSER_H
