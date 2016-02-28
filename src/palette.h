#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>

class Palette
{
public:
	Palette();
	QColor color();
	void reset();

private:
	float _hueState;
};

#endif // PALLETE_H
