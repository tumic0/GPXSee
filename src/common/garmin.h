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

inline quint8 vs(const quint8 b0)
{
	static const quint8 sizes[] = {4, 1, 2, 1, 3, 1, 2, 1};
	return sizes[b0 & 0x07];
}

inline quint8 bs(const quint8 val)
{
	return (val + 7) >> 3;
}

inline quint8 byteSize(quint32 val)
{
	quint8 ret = 0;

	do {
		ret++;
		val = val >> 8;
	} while (val != 0);

	return ret;
}

#endif // GARMIN_H
