#ifndef MAP_H
#define MAP_H

#include <QPixmap>
#include "downloader.h"


class Tile
{
public:
	Tile(const QPoint &xy, int zoom)
		{_xy = xy; _zoom = zoom;}

	int zoom() {return _zoom;}
	QPoint& xy() {return _xy;}
	QPixmap& pixmap() {return _pixmap;}

private:
	int _zoom;
	QPoint _xy;
	QPixmap _pixmap;
};


class Map : public QObject
{
	Q_OBJECT

signals:
	void loaded();

public:
	Map(const QString &name, const QString &url, QObject *parent = 0);

	const QString &name() const {return _name;}
	void loadTiles(QList<Tile> &list);

private slots:
	void emitLoaded();

private:
	QString _name;
	QString _url;
};

#endif // MAP_H
