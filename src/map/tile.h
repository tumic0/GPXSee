#ifndef TILE_H
#define TILE_H

#include <QPixmap>
#include <QPoint>
#include <QDebug>

class Tile
{
public:
	Tile(const QPoint &xy, const QVariant &zoom)
		{_xy = xy; _zoom = zoom;}

	QVariant zoom() const {return _zoom;}
	const QPoint& xy() const {return _xy;}
	QPixmap& pixmap() {return _pixmap;}

private:
	QVariant _zoom;
	QPoint _xy;
	QPixmap _pixmap;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Tile &tile)
{
	dbg.nospace() << "Tile(" << tile.zoom() << ", " << tile.xy() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // TILE_H
