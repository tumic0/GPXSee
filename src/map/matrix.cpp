#include "matrix.h"


#define abs(x) ((x)<0 ? -(x) : (x))

Matrix::~Matrix()
{
	if (isNull())
		return;

	delete[] _m;
}

Matrix::Matrix(size_t h, size_t w)
{
	_h = h; _w = w;

	if (isNull())
		_m = 0;
	else
		_m = new double[_h * _w];
}

Matrix::Matrix(const Matrix& M)
{
	_h = M._h; _w = M._w;

	if (isNull())
		_m = 0;
	else
		_m = new double[_h * _w];

	for (size_t i = 0; i < _h; i++)
		for (size_t j = 0; j < _w; j++)
			m(i,j) = M.m(i,j);
}

Matrix &Matrix::operator=(const Matrix &M)
{
	if (_h != M._h || _w != M._w) {
		if (!isNull())
			delete[] _m;

		_h = M._h; _w = M._w;
		if (isNull())
			_m = 0;
		else
			_m = new double[_h * _w];
	}

	for (size_t i = 0; i < _h; i++)
		for (size_t j = 0; j < _w; j++)
			m(i,j) = M.m(i,j);

	return *this;
}

bool Matrix::eliminate(double epsilon)
{
	size_t i, j, k, maxrow;
	double temp;


	for (i = 0; i < _h; i++) {
		maxrow = i;
		for (j = i+1; j < _h; j++)
			if (abs(m(j, i)) > abs(m(maxrow, i)))
				maxrow = j;
		for (j = 0; j < _w; j++) {
			temp = m(i, j);
			m(i, j) = m(maxrow, j);
			m(maxrow, j) = temp;
		}
		if (abs(m(i, i)) <= epsilon)
			return false;
		for (j = i+1; j<_h; j++) {
			temp = m(j, i) / m(i, i);
			for (k = i; k < _w; k++)
				m(j, k) -= m(i, k) * temp;
		}
	}
	for (i = _h-1; i < i+1; i--) {
		temp  = m(i, i);
		for (j = 0; j < i; j++)
			for (k = _w-1; k >= i; k--)
				m(j, k) -=  m(i, k) * m(j, i) / temp;
		m(i, i) /= temp;
		for (j = _h; j < _w; j++)
			m(i, j) /= temp;
	}

	return true;
}

Matrix Matrix::augemented(const Matrix &M) const
{
	if (_h != M._h)
		return Matrix();

	Matrix A(_h, _w + M._w);

	for (size_t i = 0; i < _h; i++)
		for (size_t j = 0; j < _w; j++)
			A.m(i, j) = m(i, j);

	for (size_t i = 0; i < _h; i++)
		for (size_t j = _w; j < A._w; j++)
			A.m(i, j) = M.m(i, j-_w);

	return A;
}

void Matrix::zeroize()
{
	for (size_t i = 0; i < _h * _w; i++)
		_m[i] = 0;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Matrix &matrix)
{
	dbg.nospace() << "Matrix(" << endl;
	for (size_t i = 0; i < matrix.h(); i++) {
		for (size_t j = 0; j < matrix.w(); j++)
			dbg << "\t" << matrix.m(i, j);
		dbg << endl;
	}
	dbg << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
