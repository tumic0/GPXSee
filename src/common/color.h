#ifndef COLOR_H
#define COLOR_H

#include <QColor>

namespace Color
{
	inline QRgb bgr2rgb(quint32 bgr)
	{
		quint32 b = (bgr & 0x000000FF);
		quint32 g = (bgr & 0x0000FF00) >> 8;
		quint32 r = (bgr & 0x00FF0000) >> 16;

		return (0xFF000000 | r << 16 | g << 8 | b);
	}

	inline QRgb rgb(quint32 r, quint32 g, quint32 b)
	{
		return (0xFF000000 | r << 16 | g << 8 | b);
	}
}

#endif // COLOR_H
