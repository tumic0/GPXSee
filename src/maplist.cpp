#include <QFileInfo>
#include <QDir>
#include "atlas.h"
#include "offlinemap.h"
#include "onlinemap.h"
#include "maplist.h"


Map *MapList::loadListEntry(const QByteArray &line)
{
	QList<QByteArray> list = line.split('\t');
	if (list.size() != 2)
		return 0;

	QByteArray ba1 = list[0].trimmed();
	QByteArray ba2 = list[1].trimmed();
	if (ba1.isEmpty() || ba2.isEmpty())
		return 0;

	return new OnlineMap(QString::fromUtf8(ba1.data(), ba1.size()),
	  QString::fromLatin1(ba2.data(), ba2.size()), this);
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

bool MapList::loadTba(const QString &path)
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

bool MapList::loadTar(const QString &path)
{
	Atlas *atlas = new Atlas(path, this);
	if (atlas->isValid()) {
		_maps.append(atlas);
		return true;
	} else {
		_errorString = atlas->errorString();
		delete atlas;
		OfflineMap *map = new OfflineMap(path, this);
		if (map->isValid()) {
			_maps.append(map);
			return true;
		} else {
			qWarning("%s: %s", qPrintable(path), qPrintable(_errorString));
			qWarning("%s: %s", qPrintable(path),
			  qPrintable(map->errorString()));
			_errorString = "Not a map/atlas file";
			delete map;
			return false;
		}
	}
}

bool MapList::loadFile(const QString &path)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();

	if (suffix == "txt")
		return loadList(path);
	else if (suffix == "map")
		return loadMap(path);
	else if (suffix == "tba")
		return loadTba(path);
	else if (suffix == "tar")
		return loadTar(path);
	else {
		_errorString = "Not a map/atlas file";
		return false;
	}
}

QString MapList::formats()
{
	return tr("Map files (*.map *.tba *.tar)") + ";;"
	  + tr("URL list files (*.txt)");
}

QStringList MapList::filter()
{
	QStringList filter;
	filter << "*.map" << "*.tba" << "*.tar" << "*.txt";
	return filter;
}
