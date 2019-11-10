#ifndef GARMIN_H
#define GARMIN_H

#include <QtGlobal>

inline double toWGS32(qint32 v)
{
	return (double)(((double)v / (double)(1U<<31)) * (double)180);
}

inline double toWGS24(qint32 v)
{
	return (v < 0x800000)
	  ? (double)v * 360.0 / (double)(1U<<24)
	  : (double)(v - 0x1000000) * 360.0 / (double)(1<<24);
}

#endif // GARMIN_H
