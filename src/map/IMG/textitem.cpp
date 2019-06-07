#include "textitem.h"

bool TextItem::collides(const QList<TextItem*> &list) const
{
	QRectF r1(boundingRect());

	for (int i = 0; i < list.size(); i++) {
		const TextItem* other = list.at(i);
		QRectF r2(other->boundingRect());

		if (!(r1.isEmpty() || r2.isEmpty() || !r1.intersects(r2)))
			if (other->shape().intersects(shape()))
				return true;
	}

	return false;
}
