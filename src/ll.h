#ifndef LL_H
#define LL_H

#include <QPointF>

void ll2mercator(const QPointF &src, QPointF &dst);
qreal llDistance(const QPointF &p1, const QPointF &p2);

#endif // LL_H
