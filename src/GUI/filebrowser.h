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

	void setFilter(const QStringList &filter);
	void setCurrent(const QString &path);

	QString next();
	QString prev();
	QString last();
	QString first();

	bool isLast() const;
	bool isFirst() const;

private slots:
	void reloadDirectory(const QString &path);

private:
	QFileSystemWatcher *_watcher;
	QStringList _filter;
	QFileInfoList _files;
	int _index;
};

#endif // FILEBROWSER_H
