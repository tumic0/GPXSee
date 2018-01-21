#ifndef TILE_H
#define TILE_H

#include <QPixmap>
#include <QPoint>
#include <QDebug>

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

inline QDebug operator<<(QDebug dbg, const Tile &tile)
{
	dbg.nospace() << "Tile(" << tile.zoom() << ", " << tile.xy() << ")";
	return dbg.space();
}

#endif // TILE_H
