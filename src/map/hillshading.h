#ifndef HILLSHADING_H
#define HILLSHADING_H

#include <QImage>
#include "map/matrix.h"

class HillShading
{
public:
	static QImage render(const Matrix &m, quint8 alpha = 64, double z = 0.3,
	  double azimuth = 315, double elevation = 25);
};

#endif // HILLSHADING_H
