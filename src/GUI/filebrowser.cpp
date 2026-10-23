#include <QFileSystemWatcher>
#include <QDir>
#include "common/util.h"
#include "filebrowser.h"

#ifdef Q_OS_ANDROID
#define PATH(file) file
#else // Q_OS_ANDROID
#define PATH(file) file.absoluteFilePath()
#endif // Q_OS_ANDROID

FileBrowser::FileBrowser(const QStringList &filter, QObject *parent)
  : QObject(parent)
{
#ifdef Q_OS_ANDROID
	_filter.resize(filter.size());
	for (int i = 0; i < _filter.size(); i++)
		_filter[i] = QRegularExpression::fromWildcard(filter.at(i));
#else // Q_OS_ANDROID
	_filter = filter;
	_watcher = new QFileSystemWatcher(this);
	connect(_watcher, &QFileSystemWatcher::directoryChanged, this,
	  &FileBrowser::reloadDirectory);
#endif // Q_OS_ANDROID

	_index = -1;
}

#ifdef Q_OS_ANDROID
void FileBrowser::setCurrentDir(const QString &path)
{
	if (_filter.isEmpty())
		_files = Util::listDirFiles(path);
	else {
		QStringList files(Util::listDirFiles(path));

		_files = QStringList();
		for (int i = 0; i < files.size(); i++) {
			const QString &f = files.at(i);
			QString name(f.mid(f.lastIndexOf('/') + 1));

			for (int j = 0; j < _filter.size(); j++) {
				QRegularExpressionMatch match(_filter.at(j).match(name));
				if (match.hasMatch()) {
					_files.append(f);
					break;
				}
			}
		}
	}
	std::sort(_files.begin(), _files.end());

	_index = _files.empty() ? -1 : 0;

	emit listChanged();
}
#else // Q_OS_ANDROID
void FileBrowser::setCurrent(const QString &path)
{
	QFileInfo file(path);
	QDir dir = file.absoluteDir();

	if (_files.isEmpty() || _files.last().canonicalPath()
	  != dir.canonicalPath()) {
		if (!_watcher->directories().isEmpty())
			_watcher->removePaths(_watcher->directories());
		_watcher->addPath(dir.canonicalPath());
		_files = dir.entryInfoList(_filter, QDir::Files, QDir::Name);
	}

	_index = _files.empty() ? -1 : _files.indexOf(file);
}
#endif // Q_OS_ANDROID

bool FileBrowser::isLast() const
{
	return (_files.size() > 0 && _index == _files.size() - 1);
}

bool FileBrowser::isFirst() const
{
	return (_files.size() > 0 && _index == 0);
}

QString FileBrowser::current()
{
	return (_index >= 0) ? PATH(_files.at(_index)) : QString();
}

QString FileBrowser::next()
{
	if (_index < 0 || _index == _files.size() - 1)
		return QString();

	return PATH(_files.at(++_index));
}

QString FileBrowser::prev()
{
	if (_index <= 0)
		return QString();

	return PATH(_files.at(--_index));
}

QString FileBrowser::last()
{
	if (_files.empty())
		return QString();

	_index = _files.size() - 1;
	return PATH(_files.last());
}

QString FileBrowser::first()
{
	if (_files.empty())
		return QString();

	_index = 0;
	return PATH(_files.first());
}

#ifndef Q_OS_ANDROID
void FileBrowser::reloadDirectory(const QString &path)
{
	QDir dir(path);
	QFileInfo current = (_index >= 0) ? _files.at(_index) : QFileInfo();

	_files = dir.entryInfoList(_filter, QDir::Files);
	_index = _files.empty() ? -1 : _files.indexOf(current);

	emit listChanged();
}
#endif // Q_OS_ANDROID
