#ifndef UTM_H
#define UTM_H

#include "projection.h"

namespace UTM
{
	int zone(const Coordinates &c);
	Projection::Setup setup(int zone);
}

#endif // UTM_H
