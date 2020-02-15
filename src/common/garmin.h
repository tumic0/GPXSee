#ifndef GARMIN_H
#define GARMIN_H

#include <QtGlobal>

#define LS(val, bits) ((qint32)(((quint32)(val))<<(bits)))

inline double toWGS32(qint32 v)
{
	return ((double)v / (double)(1U<<31)) * 180.0;
}

inline double toWGS24(qint32 v)
{
	return toWGS32(LS(v, 8));
}

#endif // GARMIN_H
