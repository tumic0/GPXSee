#include "coordinatesystem.h"

CoordinateSystem::CoordinateSystem(int code)
{
	switch (code) {
		case 1024:
		case 1035:
		case 1039:
		case 4400:
		case 4409:
		case 4463:
		case 4464:
		case 4465:
		case 4466:
		case 4467:
		case 4469:
		case 4470:
		case 4495:
		case 4496:
		case 4497:
		case 4498:
		case 4499:
			_axisOrder = XY;
			break;
		case 4500:
		case 4530:
		case 4531:
		case 4532:
			_axisOrder = YX;
			break;
		default:
			_axisOrder = Unknown;
			break;
	}
}
