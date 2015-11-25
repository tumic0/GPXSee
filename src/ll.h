#ifndef LL_H
#define LL_H

#include <QPointF>

#define TILE_SIZE 256
#define ZOOM_MAX 18
#define ZOOM_MIN 3

QPointF ll2mercator(const QPointF &ll);
qreal llDistance(const QPointF &p1, const QPointF &p2);
QPoint mercator2tile(const QPointF &m, int zoom);
QPointF tile2mercator(const QPoint &tile, int zoom);
int scale2zoom(qreal scale);

#endif // LL_H
