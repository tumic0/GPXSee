#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

class CoordinateSystem
{
public:
	enum AxisOrder {Unknown, XY, YX};

	CoordinateSystem() : _axisOrder(Unknown) {}
	CoordinateSystem(int code);

	bool isValid() const {return (_axisOrder != Unknown);}

	AxisOrder axisOrder() const {return _axisOrder;}

private:
	AxisOrder _axisOrder;
};

#endif // COORDINATESYSTEM_H
