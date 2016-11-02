#ifndef FORMAT_H
#define FORMAT_H

#include <QString>
#include "units.h"

class Coordinates;

namespace Format
{
	QString timeSpan(qreal time);
	QString distance(qreal value, Units units);
	QString elevation(qreal value, Units units);
	QString coordinates(const Coordinates &value);
}

#endif // FORMAT_H
