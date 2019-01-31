#include "polygon.h"

RectC Polygon::boundingRect() const
{
	if (isEmpty())
		return RectC();
	if (first().size() < 3)
		return RectC();

	RectC rect;
	for (int i = 0; i < first().size(); i++)
		rect = rect.united(first().at(i));

	return rect;
}
