#include <QFileInfo>
#include <QDir>
#include "common/range.h"
#include "atlas.h"
#include "offlinemap.h"
#include "onlinemap.h"
#include "maplist.h"


#define ZOOM_MAX 18
#define ZOOM_MIN  2

Map *MapList::loadListEntry(const QByteArray &line)
{
	int max;

	QList<QByteArray> list = line.split('\t');
	if (list.size() < 2)
		return 0;

	QByteArray ba1 = list.at(0).trimmed();
	QByteArray ba2 = list.at(1).trimmed();
	if (ba1.isEmpty() || ba2.isEmpty())
		return 0;

	if (list.size() == 3) {
		bool ok;
		max = QString(list.at(2).trimmed()).toInt(&ok);
		if (!ok)
			return 0;
	} else
		max = ZOOM_MAX;

	return new OnlineMap(QString::fromUtf8(ba1.data(), ba1.size()),
	  QString::fromLatin1(ba2.data(), ba2.size()), Range(ZOOM_MIN, max), this);
}

bool MapList::loadList(const QString &path)
{
	QFile file(path);
	QList<Map*> maps;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	int ln = 0;
	while (!file.atEnd()) {
		ln++;
		QByteArray line = file.readLine();
		Map *map = loadListEntry(line);

		if (map)
			maps.append(map);
		else {
			for (int i = 0; i < maps.count(); i++)
				delete maps.at(i);
			_errorString = QString("Invalid map list entry on line %1.")
			  .arg(QString::number(ln));
			return false;
		}
	}

	_maps += maps;

	return true;
}

bool MapList::loadMap(const QString &path)
{
	OfflineMap *map = new OfflineMap(path, this);

	if (map->isValid()) {
		_maps.append(map);
		return true;
	} else {
		_errorString = map->errorString();
		delete map;
		return false;
	}
}

bool MapList::loadAtlas(const QString &path)
{
	Atlas *atlas = new Atlas(path, this);

	if (atlas->isValid()) {
		_maps.append(atlas);
		return true;
	} else {
		_errorString = atlas->errorString();
		delete atlas;
		return false;
	}
}

bool MapList::loadFile(const QString &path, bool *atlas)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();

	if (Atlas::isAtlas(path)) {
		if (atlas)
			*atlas = true;
		return loadAtlas(path);
	} else if (suffix == "txt") {
		if (atlas)
			*atlas = false;
		return loadList(path);
	} else {
		if (atlas)
			*atlas = false;
		return loadMap(path);
	}
}

bool MapList::loadDir(const QString &path)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	md.setSorting(QDir::DirsLast);
	QFileInfoList ml = md.entryInfoList();
	bool atlas;

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		QString suffix = fi.suffix().toLower();

		if (fi.isDir() && fi.fileName() != "set") {
			if (!loadDir(fi.absoluteFilePath()))
				return false;
		} else if (filter().contains("*." + suffix)) {
			if (!loadFile(fi.absoluteFilePath(), &atlas)) {
				_errorString.prepend(QString("%1: ")
				  .arg(fi.canonicalFilePath()));
				return false;
			}
			if (atlas)
				break;
		}
	}

	return true;
}

void MapList::clear()
{
	for (int i = 0; i < _maps.count(); i++)
		delete _maps.at(i);
	_maps.clear();
}

QString MapList::formats()
{
	return tr("Supported files (*.txt *.map *.tba *.tar *.tif *.tiff)") + ";;"
	  + tr("Offline maps (*.map *.tba *.tar *.tif *.tiff)") + ";;"
	  + tr("Online map lists (*.txt)");
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.map" << "*.tba" << "*.tar" << "*.txt" << "*.tif" << "*.tiff";
	return filter;
}
