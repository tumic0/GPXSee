#ifndef CRS_H
#define CRS_H

#include "projection.h"

namespace CRS
{
	Projection projection(const QString &crs);
	Projection projection(int id);
	Projection projection(int gcsId, int projId);
}

#endif // CRS_H
