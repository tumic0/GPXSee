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

static double avg(const MatrixD &m, int pi, int pj, int r)
{
	int col, row, cnt = 0;
	double sum = 0;

	if (r >= qMax(m.h(), m.w()))
		return NAN;

	row = pi - r;
	if (row >= 0) {
		for (int j = qMax(pj - r, 0); j < qMin(pj + r, m.w()); j++) {
			if (!std::isnan(m.at(row, j))) {
				sum += m.at(row, j);
				cnt++;
			}
		}
	}
	row = pi + r;
	if (row < m.h()) {
		for (int j = qMax(pj - r, 0); j < qMin(pj + r, m.w()); j++) {
			if (!std::isnan(m.at(row, j))) {
				sum += m.at(row, j);
				cnt++;
			}
		}
	}
	col = pj - r;
	if (col >= 0) {
		for (int i = qMax(pi - r + 1, 0); i < qMin(pi + r - 1, m.h()); i++) {
			if (!std::isnan(m.at(i, col))) {
				sum += m.at(i, col);
				cnt++;
			}
		}
	}
	col = pj + r;
	if (col < m.w()) {
		for (int i = qMax(pi - r + 1, 0); i < qMin(pi + r - 1, m.h()); i++) {
			if (!std::isnan(m.at(i, col))) {
				sum += m.at(i, col);
				cnt++;
			}
		}
	}

	if (cnt)
		return sum / cnt;
	else
		return avg(m, pi, pj, r + 1);
}

static bool fillNANs(const MatrixD &m, MatrixD &src)
{
	bool hasNAN = false;

	for (int i = 0; i < m.h(); i++) {
		for (int j = 0; j < m.w(); j++) {
			if (std::isnan(m.at(i, j))) {
				hasNAN = true;

				double val = avg(src, i, j, 1);
				if (std::isnan(val))
					return false;
				else
					src.at(i, j) = val;
			}
		}
	}

	return hasNAN;
}

static void revertNANs(const MatrixD &m, MatrixD &dst)
{
	for (int i = 0; i < m.size(); i++)
		if (std::isnan(m.at(i)))
			dst.at(i) = NAN;
}

MatrixD Filter::blur(const MatrixD &m, int radius)
{
	MatrixD src(m);
	MatrixD dst(m.h(), m.w());
	bool hasNAN = fillNANs(m, src);

	gaussBlur4(src, dst, radius);

	if (hasNAN)
		revertNANs(m, dst);

	return dst;
}
