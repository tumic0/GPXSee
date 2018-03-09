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

	bool loadFile(const QString &path);
	bool loadDir(const QString &path);
	void clear();

	const QList<Map*> &maps() const {return _maps;}
	const QString &errorString() const {return _errorString;}

	static QString formats();
	static QStringList filter();

private:
	bool loadFile(const QString &path, bool *atlas, bool dir);
	bool loadDirR(const QString &path);

	Map *loadListEntry(const QByteArray &line);

	bool loadSource(const QString &path, bool dir);
	bool loadMap(const QString &path, bool dir);
	bool loadAtlas(const QString &path, bool dir);

	QList<Map*> _maps;
	QString _errorString;
};

#endif // MAPLIST_H
