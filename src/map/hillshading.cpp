#include <cmath>
#include "hillshading.h"

struct Constants
{
	double a1;
	double a2;
	double a3;
};

struct SubMatrix
{
	double z1;
	double z2;
	double z3;
	double z4;
	double z6;
	double z7;
	double z8;
	double z9;
};

struct Derivatives
{
	double dzdx;
	double dzdy;
};

static void getConstants(double azimuth, double elevation, Constants &c)
{
	double alpha = (M_PI / 180.0) * azimuth;
	double beta = (M_PI / 180.0) * elevation;

	c.a1 = sin(beta);
	c.a2 = cos(beta) * sin(alpha);
	c.a3 = cos(beta) * cos(alpha);
}

static void getDerivativesHorn(const SubMatrix &sm, double z, Derivatives &d)
{
	d.dzdx = (z * (sm.z3 + 2 * sm.z6 + sm.z9 - sm.z1 - 2 * sm.z4 - sm.z7)) / 8;
	d.dzdy = (z * (sm.z1 + 2 * sm.z2 + sm.z3 - sm.z7 - 2 * sm.z8 - sm.z9)) / 8;
}

static void getSubmatrix(int x, int y, const Matrix &m, SubMatrix &sm)
{
	int left = x - 1;
	int right = x + 1;
	int top = y - 1;
	int bottom = y + 1;

	sm.z1 = m.m(top, left);
	sm.z2 = m.m(top, x);
	sm.z3 = m.m(top, right);
	sm.z4 = m.m(y, left);
	sm.z6 = m.m(y, right);
	sm.z7 = m.m(bottom, left);
	sm.z8 = m.m(bottom, x);
	sm.z9 = m.m(bottom, right);
}

QImage HillShading::render(const Matrix &m, quint8 alpha, double z,
  double azimuth, double elevation)
{
	QImage img(m.w() - 2, m.h() - 2, QImage::Format_ARGB32_Premultiplied);
	uchar *bits = img.bits();
	int bpl = img.bytesPerLine();

	Constants c;
	SubMatrix sm;
	Derivatives d;

	getConstants(azimuth, elevation, c);

	for (int y = 1; y < m.h() - 1; y++) {
		for (int x = 1; x < m.w() - 1; x++) {
			getSubmatrix(x, y, m, sm);
			getDerivativesHorn(sm, z, d);

			double L = (c.a1 - c.a2 * d.dzdx - c.a3 * d.dzdy)
			  / sqrt(1.0 + d.dzdx * d.dzdx + d.dzdy * d.dzdy);

			quint32 pixel;
			if (std::isnan(L))
				pixel = 0;
			else {
				quint8 val = (L < 0) ? 0 : L * alpha;
				pixel = alpha<<24 | val<<16 | val<<8 | val;
			}

			*(quint32*)(bits + (y - 1) * bpl + (x - 1) * 4) = pixel;
		}
	}

	return img;
}
