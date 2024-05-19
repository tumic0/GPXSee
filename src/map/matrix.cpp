#include "matrix.h"

#define abs(x) ((x)<0 ? -(x) : (x))

bool MatrixD::eliminate(double epsilon)
{
	double temp;

	for (int i = 0; i < _h; i++) {
		int maxrow = i;
		for (int j = i+1; j < _h; j++)
			if (abs(at(j, i)) > abs(at(maxrow, i)))
				maxrow = j;
		for (int j = 0; j < _w; j++) {
			temp = at(i, j);
			at(i, j) = at(maxrow, j);
			at(maxrow, j) = temp;
		}
		if (abs(at(i, i)) <= epsilon)
			return false;
		for (int j = i+1; j<_h; j++) {
			temp = at(j, i) / at(i, i);
			for (int k = i; k < _w; k++)
				at(j, k) -= at(i, k) * temp;
		}
	}
	for (int i = _h-1; i >= 0; i--) {
		temp  = at(i, i);
		for (int j = 0; j < i; j++)
			for (int k = _w-1; k >= i; k--)
				at(j, k) -=  at(i, k) * at(j, i) / temp;
		at(i, i) /= temp;
		for (int j = _h; j < _w; j++)
			at(i, j) /= temp;
	}

	return true;
}

MatrixD MatrixD::augemented(const MatrixD &M) const
{
	if (_h != M._h)
		return MatrixD();

	MatrixD A(_h, _w + M._w);

	for (int i = 0; i < _h; i++)
		for (int j = 0; j < _w; j++)
			A.at(i, j) = at(i, j);

	for (int i = 0; i < _h; i++)
		for (int j = _w; j < A._w; j++)
			A.at(i, j) = M.at(i, j-_w);

	return A;
}
