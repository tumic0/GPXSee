#ifndef MISC_H
#define MISC_H

#include <QString>
#include "units.h"

double niceNum(double x, int round);
QString timeSpan(qreal time);
QString distance(qreal value, Units units);

#endif // MISC_H
