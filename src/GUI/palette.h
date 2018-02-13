#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>
#include <QDebug>

class Palette
{
public:
	Palette(const QColor &color = Qt::blue, qreal shift = 0.62);

	QColor color() const {return QColor::fromHsvF(_h, _s, _v, _a).toRgb();}
	qreal shift() const {return _shift;}
	void setColor(const QColor &color);
	void setShift(qreal shift);

	QColor nextColor();
	void reset();

	bool operator==(const Palette &other) const
	  {return (_h == other._h && _s == other._s && _v == other._v
	  && _a == other._a && _shift == other._shift);}
	bool operator!=(const Palette &other) const
	  {return !(*this == other);}

private:
	qreal _h, _s, _v, _a, _shift;
	qreal _state;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Palette &palette);
#endif // QT_NO_DEBUG

#endif // PALLETE_H
