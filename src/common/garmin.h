#ifndef GARMIN_H
#define GARMIN_H

#include <QtGlobal>

inline double toWGS32(qint32 v)
{
	return (double)(((double)v / (double)(1U<<31)) * (double)180);
}

inline double toWGS24(qint32 v)
{
	return toWGS32(v<<8);
}

#endif // GARMIN_H
