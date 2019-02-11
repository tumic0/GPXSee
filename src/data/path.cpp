#include "path.h"

bool Path::isValid() const
{
	if (isEmpty())
		return false;
	for (int i = 0; i < size(); i++)
		if (at(i).size() < 2)
			return false;
	return true;
}

RectC Path::boundingRect() const
{
	RectC ret;

	if (!isValid())
		return ret;

	for (int i = 0; i < size(); i++) {
		const PathSegment &segment = at(i);
		for (int j = 0; j < segment.size(); j++)
			ret = ret.united(segment.at(j).coordinates());
	}

	return ret;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const PathPoint &point)
{
	dbg.nospace() << "PathPoint(" << point.distance() << ", "
	  << point.coordinates() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
