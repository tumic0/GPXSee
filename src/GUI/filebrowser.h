#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QObject>
#include <QFileInfo>
#include <QStringList>
#include <QRegularExpression>

class QFileSystemWatcher;

class FileBrowser : public QObject
{
	Q_OBJECT

public:
	FileBrowser(const QStringList &filter, QObject *parent = 0);

#ifdef Q_OS_ANDROID
	void setCurrentDir(const QString &path);
#else // Q_OS_ANDROID
	void setCurrent(const QString &path);
#endif // Q_OS_ANDROID

	QString current();
	QString next();
	QString prev();
	QString last();
	QString first();

	bool isLast() const;
	bool isFirst() const;

signals:
	void listChanged();

#ifndef Q_OS_ANDROID
private slots:
	void reloadDirectory(const QString &path);
#endif // Q_OS_ANDROID

private:
	int _index;

#ifdef Q_OS_ANDROID
	QStringList _files;
	QList<QRegularExpression> _filter;
#else // Q_OS_ANDROID
	QStringList _filter;
	QFileInfoList _files;
	QFileSystemWatcher *_watcher;
#endif // Q_OS_ANDROID
};

#endif // FILEBROWSER_H
