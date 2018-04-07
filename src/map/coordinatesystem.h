#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

#include <QDebug>

class CoordinateSystem
{
public:
	enum AxisOrder {Unknown, XY, YX};

	CoordinateSystem() : _axisOrder(Unknown) {}
	CoordinateSystem(AxisOrder axisOrder) : _axisOrder(axisOrder) {}
	CoordinateSystem(int code);

	bool isValid() const {return (_axisOrder != Unknown);}

	AxisOrder axisOrder() const {return _axisOrder;}

private:
	AxisOrder _axisOrder;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const CoordinateSystem &cs);
#endif // QT_NO_DEBUG

#endif // COORDINATESYSTEM_H
