#ifndef TILE_H
#define TILE_H

#include <QVariant>
#include <QPixmap>
#include <QPoint>
#include <QDebug>
#include "rectd.h"

class Tile
{
public:
	Tile() {}
	Tile(const QPoint &xy, const QVariant &zoom, int scaledSize = 0,
	  const RectD &bbox = RectD()) : _xy(xy), _zoom(zoom),
	  _scaledSize(scaledSize), _bbox(bbox) {}

	const QVariant &zoom() const {return _zoom;}
	const QPoint &xy() const {return _xy;}
	const RectD &bbox() const {return _bbox;}
	int scaledSize() const {return _scaledSize;}
	QPixmap& pixmap() {return _pixmap;}

private:
	QPoint _xy;
	QVariant _zoom;
	int _scaledSize;
	RectD _bbox;
	QPixmap _pixmap;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Tile &tile)
{
	dbg.nospace() << "Tile(" << tile.zoom() << ", " << tile.xy() << ", "
	  << tile.bbox() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // TILE_H
