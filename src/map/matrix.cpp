#include "matrix.h"


#define abs(x) ((x)<0 ? -(x) : (x))

Matrix::Matrix(int h, int w)
{
	_h = h;
	_w = w;
	_m.resize(_h * _w);
}

bool Matrix::eliminate(double epsilon)
{
	double temp;

	for (int i = 0; i < _h; i++) {
		int maxrow = i;
		for (int j = i+1; j < _h; j++)
			if (abs(m(j, i)) > abs(m(maxrow, i)))
				maxrow = j;
		for (int j = 0; j < _w; j++) {
			temp = m(i, j);
			m(i, j) = m(maxrow, j);
			m(maxrow, j) = temp;
		}
		if (abs(m(i, i)) <= epsilon)
			return false;
		for (int j = i+1; j<_h; j++) {
			temp = m(j, i) / m(i, i);
			for (int k = i; k < _w; k++)
				m(j, k) -= m(i, k) * temp;
		}
	}
	for (int i = _h-1; i >= 0; i--) {
		temp  = m(i, i);
		for (int j = 0; j < i; j++)
			for (int k = _w-1; k >= i; k--)
				m(j, k) -=  m(i, k) * m(j, i) / temp;
		m(i, i) /= temp;
		for (int j = _h; j < _w; j++)
			m(i, j) /= temp;
	}

	return true;
}

Matrix Matrix::augemented(const Matrix &M) const
{
	if (_h != M._h)
		return Matrix();

	Matrix A(_h, _w + M._w);

	for (int i = 0; i < _h; i++)
		for (int j = 0; j < _w; j++)
			A.m(i, j) = m(i, j);

	for (int i = 0; i < _h; i++)
		for (int j = _w; j < A._w; j++)
			A.m(i, j) = M.m(i, j-_w);

	return A;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Matrix &matrix)
{
	dbg.nospace() << "Matrix(" << "\n";
	for (int i = 0; i < matrix.h(); i++) {
		for (int j = 0; j < matrix.w(); j++)
			dbg << "\t" << matrix.m(i, j);
		dbg << "\n";
	}
	dbg << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
