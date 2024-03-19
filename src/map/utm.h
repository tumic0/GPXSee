#ifndef UTM_H
#define UTM_H

#include "conversion.h"

class Coordinates;

namespace UTM
{
	int zone(const Coordinates &c);
	Conversion::Setup setup(int zone);
}

#endif // UTM_H
