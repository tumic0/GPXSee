#ifndef MAPLIST_H
#define MAPLIST_H

#include <QObject>
#include <QString>

class Map;

class MapList : public QObject
{
	Q_OBJECT

public:
	MapList(QObject *parent = 0) : QObject(parent) {}

	bool loadFile(const QString &path, bool *terminate = 0);
	bool loadDir(const QString &path);

	const QList<Map*> &maps() const {return _maps;}

	const QString &errorString() const {return _errorString;}
	const QString &errorPath() const {return _errorPath;}

	static QString formats();
	static QStringList filter();

private:
	Map *loadSource(const QString &path);
	bool loadMap(Map *map, const QString &path);

	QList<Map*> _maps;
	QString _errorString;
	QString _errorPath;
};

#endif // MAPLIST_H
