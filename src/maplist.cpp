#include <QFileInfo>
#include <QDir>
#include "atlas.h"
#include "offlinemap.h"
#include "onlinemap.h"
#include "maplist.h"


bool MapList::loadListEntry(const QByteArray &line)
{
	QList<QByteArray> list = line.split('\t');
	if (list.size() != 2)
		return false;

	QByteArray ba1 = list[0].trimmed();
	QByteArray ba2 = list[1].trimmed();
	if (ba1.isEmpty() || ba2.isEmpty())
		return false;

	_maps.append(new OnlineMap(QString::fromUtf8(ba1.data(), ba1.size()),
	  QString::fromLatin1(ba2.data(), ba2.size()), this));

	return true;
}

bool MapList::loadList(const QString &path)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	int ln = 0;
	while (!file.atEnd()) {
		ln++;
		QByteArray line = file.readLine();

		if (!loadListEntry(line)) {
			_errorString = QString("Invalid map list entry on line %1.")
			  .arg(QString::number(ln));
			return false;
		}
	}

	return true;
}

bool MapList::loadMap(const QString &path)
{
	QFileInfo fi(path);
	QString suffix = fi.suffix().toLower();

	if (suffix == "map") {
		OfflineMap *om = new OfflineMap(path, this);
		if (om->isValid()) {
			_maps.append(om);
			return true;
		} else {
			_errorString = om->errorString();
			delete om;
			return false;
		}
	} else if (suffix == "tba") {
		Atlas *atlas = new Atlas(path, this);
		if (atlas->isValid()) {
			_maps.append(atlas);
			return true;
		} else {
			_errorString = atlas->errorString();
			delete atlas;
			return false;
		}
	} else if (suffix == "tar") {
		Atlas *atlas = new Atlas(path, this);
		if (atlas->isValid()) {
			_maps.append(atlas);
			return true;
		} else {
			_errorString = atlas->errorString();
			delete atlas;
			OfflineMap *om = new OfflineMap(path, this);
			if (om->isValid()) {
				_maps.append(om);
				return true;
			} else {
				qWarning("%s: %s", qPrintable(path), qPrintable(_errorString));
				qWarning("%s: %s", qPrintable(path),
				  qPrintable(om->errorString()));
				_errorString = "Not a map/atlas file";
				delete om;
				return false;
			}
		}
	} else {
		_errorString = "Not a map/atlas file";
		return false;
	}
}
