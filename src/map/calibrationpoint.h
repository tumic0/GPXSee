#ifndef CALIBRATIONPOINT_H
#define CALIBRATIONPOINT_H

#include <QDebug>
#include "transform.h"
#include "projection.h"

class CalibrationPoint {
public:
	CalibrationPoint() {}
	CalibrationPoint(PointD xy, PointD pp) : _xy(xy), _pp(pp) {}
	CalibrationPoint(PointD xy, Coordinates c) : _xy(xy), _ll(c) {}

	bool isValid() const
	{
		return !(_xy.isNull() || (_pp.isNull() && !_ll.isValid()));
	}

	ReferencePoint rp(const Projection &projection) const
	{
		return (_pp.isNull())
		  ? ReferencePoint(_xy, projection.ll2xy(_ll))
		  : ReferencePoint(_xy, _pp);
	}

	friend QDebug operator<<(QDebug dbg, const CalibrationPoint &p);

private:
	PointD _xy;
	PointD _pp;
	Coordinates _ll;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const CalibrationPoint &p)
{
	dbg.nospace() << "CalibrationPoint(" << p._xy << ", " << p._pp << ", "
	  << p._ll << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // CALIBRATIONPOINT_H
