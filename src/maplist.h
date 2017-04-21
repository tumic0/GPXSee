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

	bool loadMap(const QString &path);
	bool loadList(const QString &path);

	QList<Map*> &maps() {return _maps;}
	const QString &errorString() const {return _errorString;}

private:
	bool loadListEntry(const QByteArray &line);

	QList<Map*> _maps;
	QString _errorString;
};

#endif // MAPLIST_H
