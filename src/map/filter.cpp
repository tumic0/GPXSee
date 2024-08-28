#include <cmath>
#include "filter.h"

static QVector<int> boxesForGauss(double sigma, int n)
{
	double wIdeal = sqrt((12 * sigma * sigma / n) + 1);
	int wl = floor(wIdeal);
	if (wl % 2 == 0)
		wl--;
	int wu = wl + 2;

	double mIdeal = (12 * sigma*sigma - n * wl * wl - 4 * n * wl - 3 * n)
	  / (-4 * wl - 4);
	int m = round(mIdeal);

	QVector<int> sizes(n);
	for (int i = 0; i < n; i++)
		sizes[i] = i < m ? wl : wu;

	return sizes;
}

static void boxBlurH4(const MatrixD &src, MatrixD &dst, int r)
{
	double iarr = 1.0 / (r + r + 1);

	for (int i = 0; i < src.h(); i++) {
		int ti = i * src.w(), li = ti, ri = ti + r;
		double fv = src.at(ti);
		double lv = src.at(ti + src.w() - 1);
		double val = (r + 1) * fv;

		for (int j = 0; j < r; j++)
			val += src.at(ti + j);
		for (int j = 0; j <= r; j++) {
			val += src.at(ri++) - fv;
			dst.at(ti++) = val * iarr;
		}
		for (int j = r + 1; j < src.w() - r; j++) {
			val += src.at(ri++) - src.at(li++);
			dst.at(ti++) = val * iarr;
		}
		for (int j = src.w() - r; j < src.w(); j++) {
			val += lv - src.at(li++);
			dst.at(ti++) = val * iarr;
		}
	}
}

static void boxBlurT4(const MatrixD &src, MatrixD &dst, int r)
{
	double iarr = 1.0 / (r + r + 1);

	for (int i = 0; i < src.w(); i++) {
		int ti = i, li = ti, ri = ti + r * src.w();
		double fv = src.at(ti);
		double lv = src.at(ti + src.w() * (src.h() - 1));
		double val = (r + 1) * fv;

		for (int j = 0; j < r; j++)
			val += src.at(ti + j * src.w());
		for (int j = 0; j <= r; j++) {
			val += src.at(ri) - fv;
			dst.at(ti) = val * iarr;
			ri += src.w(); ti += src.w();
		}
		for (int j = r + 1; j < src.h() - r; j++) {
			val += src.at(ri) - src.at(li);
			dst.at(ti) = val * iarr;
			li += src.w(); ri += src.w(); ti += src.w();
		}
		for (int j = src.h() - r; j < src.h(); j++) {
			val += lv - src.at(li);
			dst.at(ti) = val * iarr;
			li += src.w(); ti += src.w();
		}
	}
}

static void boxBlur4(MatrixD &src, MatrixD &dst, int r)
{
	for (int i = 0; i < src.size(); i++)
		dst.at(i) = src.at(i);

	boxBlurH4(dst, src, r);
	boxBlurT4(src, dst, r);
}

static void gaussBlur4(MatrixD &src, MatrixD &dst, int r)
{
	QVector<int> bxs(boxesForGauss(r, 3));

	boxBlur4(src, dst, (bxs.at(0) - 1) / 2);
	boxBlur4(dst, src, (bxs.at(1) - 1) / 2);
	boxBlur4(src, dst, (bxs.at(2) - 1) / 2);
}

static int hasNANs(const MatrixD &m)
{
	for (int i = 0; i < m.size(); i++)
		if (std::isnan(m.at(i)))
			return i;

	return -1;
}

MatrixD Filter::blur(const MatrixD &m, int radius)
{
	MatrixD src(m);
	MatrixD dst(m.h(), m.w());
	int firstNAN = hasNANs(m);

	if (firstNAN >= 0) {
		// https://stackoverflow.com/a/36307291
		MatrixD z(m.h(), m.w(), 1);
		MatrixD zdst(m.h(), m.w());

		for (int i = firstNAN; i < m.size(); i++) {
			if (std::isnan(m.at(i))) {
				z.at(i) = 0;
				src.at(i) = 0;
			}
		}

		gaussBlur4(z, zdst, radius);
		gaussBlur4(src, dst, radius);

		for (int i = 0; i < m.size(); i++)
			dst.at(i) = dst.at(i) / zdst.at(i);

		for (int i = firstNAN; i < m.size(); i++)
			if (std::isnan(m.at(i)))
				dst.at(i) = NAN;
	} else
		gaussBlur4(src, dst, radius);

	return dst;
}
