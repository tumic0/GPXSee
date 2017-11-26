#ifndef MAPLIST_H
#define MAPLIST_H

#include <QObject>
#include <QString>
#include "map.h"

class MapList : public QObject
{
	Q_OBJECT

public:
	MapList(QObject *parent = 0) : QObject(parent) {}

	bool loadFile(const QString &path);

	const QList<Map*> &maps() const {return _maps;}
	const QString &errorString() const {return _errorString;}

	static QString formats();
	static QStringList filter();

private:
	Map *loadListEntry(const QByteArray &line);

	bool loadList(const QString &path);
	bool loadMap(const QString &path);
	bool loadTba(const QString &path);
	bool loadTar(const QString &path);

	QList<Map*> _maps;
	QString _errorString;
};

#endif // MAPLIST_H
