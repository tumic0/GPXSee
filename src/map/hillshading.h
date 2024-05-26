#ifndef HILLSHADING_H
#define HILLSHADING_H

#include <QImage>
#include "map/matrix.h"

class HillShading
{
public:
	static QImage render(const MatrixD &m, int extend);

	static int blur() {return _blur;}

	static void setAlpha(int alpha) {_alpha = alpha;}
	static void setBlur(int blur) {_blur = blur;}
	static void setAzimuth(int azimuth) {_azimuth = azimuth;}
	static void setAltitude(int altitude) {_altitude = altitude;}
	static void setZFactor(double z) {_z = z;}

private:
	static int _alpha;
	static int _blur;
	static int _azimuth;
	static int _altitude;
	static double _z;
};

#endif // HILLSHADING_H
