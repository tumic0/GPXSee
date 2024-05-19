#ifndef MATRIX_H
#define MATRIX_H

#include <cfloat>
#include <QVector>
#include <QDebug>

template <class T>
class Matrix
{
public:
	Matrix() {_h = 0; _w = 0;}
	Matrix(int h, int w) : _h(h), _w(w) {_m.resize(_h * _w);}
	Matrix(int h, int w, const T &val) : _m(h * w, val), _h(h), _w(w) {}

	int h() const {return _h;}
	int w() const {return _w;}
	T &at(int n) {return _m[n];}
	T &at(int i, int j) {return _m[_w * i + j];}
	T const &at(int i, int j) const {return _m.at(_w * i + j);}

	bool isNull() const {return (_h == 0 || _w == 0);}
	int size() const {return _m.size();}

protected:
	QVector<T> _m;
	int _h, _w;
};

class MatrixD : public Matrix<double>
{
public:
	MatrixD() : Matrix<double>() {}
	MatrixD(int h, int w) : Matrix<double>(h, w) {}

	bool eliminate(double epsilon = DBL_EPSILON);
	MatrixD augemented(const MatrixD &M) const;
};

#ifndef QT_NO_DEBUG
template <class T>
inline QDebug operator<<(QDebug dbg, const Matrix<T> &matrix)
{
	dbg.nospace() << "Matrix(" << "\n";
	for (int i = 0; i < matrix.h(); i++) {
		for (int j = 0; j < matrix.w(); j++)
			dbg << "\t" << matrix.at(i, j);
		dbg << "\n";
	}
	dbg << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // MATRIX_H
