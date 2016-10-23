#ifndef MISC_H
#define MISC_H

#include <QString>
#include <QPoint>
#include "coordinates.h"
#include "units.h"

double niceNum(double x, int round);

QString timeSpan(qreal time);
QString distance(qreal value, Units units);
QString elevation(qreal value, Units units);
QString coordinates(const Coordinates &value);

#endif // MISC_H
