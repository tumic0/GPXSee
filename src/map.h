#ifndef MAP_H
#define MAP_H

#include <QPixmap>

class Tile
{
public:
	Tile(const QPoint &xy, int zoom)
		{_xy = xy; _zoom = zoom;}

	int zoom() const {return _zoom;}
	const QPoint& xy() const {return _xy;}
	QPixmap& pixmap() {return _pixmap;}

private:
	int _zoom;
	QPoint _xy;
	QPixmap _pixmap;
};


class Map : public QObject
{
	Q_OBJECT

public:
	Map(QObject *parent = 0, const QString &name = QString(),
	  const QString &url = QString());

	const QString &name() const {return _name;}
	void loadTiles(QList<Tile> &list, bool block);
	void clearCache();

signals:
	void loaded();

private slots:
	void emitLoaded();

private:
	QString tileUrl(const Tile &tile);
	QString tileFile(const Tile &tile);
	bool loadTileFile(Tile &tile, const QString &file);
	void fillTile(Tile &tile);

	void loadTilesAsync(QList<Tile> &list);
	void loadTilesSync(QList<Tile> &list);

	QString _name;
	QString _url;
};

#endif // MAP_H
