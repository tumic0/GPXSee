#ifndef UNITS_H
#define UNITS_H

inline double toWGS84(qint32 coord)
{
	return (coord < 0x800000)
	  ? (double)coord * 360.0 / (double)(1<<24)
	  : (double)(coord - 0x1000000) * 360.0 / (double)(1<<24);
}

#endif // UNITS_H
