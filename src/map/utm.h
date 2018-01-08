#ifndef UTM_H
#define UTM_H

#include "projection.h"

class UTM
{
public:
	static int zone(const Coordinates &c);
	static Projection::Setup setup(int zone);
};

#endif // UTM_H
