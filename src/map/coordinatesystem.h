#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

#include "axisorder.h"

class CoordinateSystem
{
public:
	CoordinateSystem() : _axisOrder(Unknown) {}
	CoordinateSystem(int code);

	bool isValid() const {return (_axisOrder != Unknown);}

	AxisOrder axisOrder() const {return _axisOrder;}

private:
	AxisOrder _axisOrder;
};

#endif // COORDINATESYSTEM_H
