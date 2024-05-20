#ifndef HILLSHADING_H
#define HILLSHADING_H

#include <QImage>
#include "map/matrix.h"

class HillShading
{
public:
	static QImage render(const MatrixD &m, int extend, quint8 alpha = 96,
	  double z = 0.6, double azimuth = 315, double elevation = 45);
};

#endif // HILLSHADING_H
