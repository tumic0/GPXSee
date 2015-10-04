#ifndef COLORSHOP_H
#define COLORSHOP_H

#include <QColor>

class ColorShop
{
public:
	ColorShop();
	QColor color();
	void reset();

private:
	float _hueState;
};

#endif // COLORSHOP_H
