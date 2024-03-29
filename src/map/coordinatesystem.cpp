#include "coordinatesystem.h"

CoordinateSystem::CoordinateSystem(int code)
{
	switch (code) {
		case 1024:
		case 1035:
		case 1039:
		case 4400:
		case 4402:
		case 4404:
		case 4405:
		case 4409:
		case 4463:
		case 4464:
		case 4465:
		case 4466:
		case 4467:
		case 4468:
		case 4469:
		case 4470:
		case 4471:
		case 4472:
		case 4473:
		case 4474:
		case 4475:
		case 4476:
		case 4477:
		case 4478:
		case 4479:
		case 4480:
		case 4481:
		case 4482:
		case 4483:
		case 4484:
		case 4485:
		case 4486:
		case 4487:
		case 4488:
		case 4489:
		case 4490:
		case 4495:
		case 4496:
		case 4497:
		case 4498:
		case 4499:
			_axisOrder = XY;
			break;
		case 1053:
		case 4500:
		case 4530:
		case 4531:
		case 4532:
		case 6501:
			_axisOrder = YX;
			break;
		default:
			_axisOrder = Unknown;
			break;
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const CoordinateSystem &cs)
{
	QString ao;

	switch (cs.axisOrder()) {
		case CoordinateSystem::XY:
			ao = "XY";
			break;
		case CoordinateSystem::YX:
			ao = "YX";
			break;
		default:
			ao = "Unknown";
	}

	dbg.nospace() << "CoordinateSystem(" << ao << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
