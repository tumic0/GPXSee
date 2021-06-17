#ifndef CRS_H
#define CRS_H

#include "projection.h"

namespace CRS
{
	Projection projection(const QString &crs);
	Projection projection(int id);
}

#endif // CRS_H
