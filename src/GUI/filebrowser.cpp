#include <QFileSystemWatcher>
#include <QDir>
#include "filebrowser.h"


FileBrowser::FileBrowser(QObject *parent) : QObject(parent)
{
	_watcher = new QFileSystemWatcher(this);

	connect(_watcher, SIGNAL(directoryChanged(const QString &)), this,
	  SLOT(reloadDirectory(const QString &)));

	_index = -1;
}

void FileBrowser::setCurrent(const QString &path)
{
	QFileInfo file(path);
	QDir dir = file.absoluteDir();

	if (_files.isEmpty() || _files.last().canonicalPath()
	  != dir.canonicalPath()) {
		if (!_watcher->directories().isEmpty())
			_watcher->removePaths(_watcher->directories());
		_watcher->addPath(dir.canonicalPath());
		_files = dir.entryInfoList(_filter, QDir::Files);
	}

	_index = _files.empty() ? -1 : _files.indexOf(file);
}

void FileBrowser::setFilter(const QStringList &filter)
{
	_filter = filter;
	if (!_files.isEmpty())
		reloadDirectory(_files.last().canonicalPath());
}

bool FileBrowser::isLast() const
{
	return (_files.size() > 0 && _index == _files.size() - 1);
}

bool FileBrowser::isFirst() const
{
	return (_files.size() > 0 && _index == 0);
}

QString FileBrowser::next()
{
	if (_index < 0 || _index == _files.size() - 1)
		return QString();

	return _files.at(++_index).absoluteFilePath();
}

QString FileBrowser::prev()
{
	if (_index <= 0)
		return QString();

	return _files.at(--_index).absoluteFilePath();
}

QString FileBrowser::last()
{
	if (_files.empty())
		return QString();

	_index = _files.size() - 1;
	return _files.last().absoluteFilePath();
}

QString FileBrowser::first()
{
	if (_files.empty())
		return QString();

	_index = 0;
	return _files.first().absoluteFilePath();
}

void FileBrowser::reloadDirectory(const QString &path)
{
	QDir dir(path);
	QFileInfo current = _files.at(_index);

	_files = dir.entryInfoList(_filter, QDir::Files);
	_index = _files.empty() ? -1 : _files.indexOf(current);
}
