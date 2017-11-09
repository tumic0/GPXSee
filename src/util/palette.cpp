#include "palette.h"

Palette::Palette(const QColor &color, qreal shift)
{
	_h = 0; _s = 0; _v = 0; _a = 1.0;

	setColor(color);
	setShift(shift);

	_state = _h;
}

void Palette::setColor(const QColor &color)
{
	if (color.isValid())
		color.getHsvF(&_h, &_s, &_v, &_a);
}

void Palette::setShift(qreal shift)
{
	if (shift >= 0 && shift <= 1.0)
		_shift = shift;
}

QColor Palette::nextColor()
{
	QColor ret = QColor::fromHsvF(_state, _s, _v, _a);

	_state += _shift;
	_state -= (int) _state;

	return ret;
}

void Palette::reset()
{
	_state = _h;
}

QDebug operator<<(QDebug dbg, const Palette &palette)
{
	dbg.nospace() << "Palette(" << palette.color() << ", " << palette.shift()
	  << ")";
	return dbg.space();
}
