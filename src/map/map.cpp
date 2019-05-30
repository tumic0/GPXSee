#include <QLineF>
#include "map.h"

qreal Map::resolution(const QRectF &rect)
{
	qreal cy = rect.center().y();
	QPointF cl(rect.left(), cy);
	QPointF cr(rect.right(), cy);

	qreal ds = xy2ll(cl).distanceTo(xy2ll(cr));
	qreal ps = QLineF(cl, cr).length();

	return ds/ps;
}
