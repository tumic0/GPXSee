#ifndef MATRIX_H
#define MATRIX_H

#include <cfloat>
#include <QVector>
#include <QDebug>

class Matrix
{
public:
	Matrix() {_h = 0; _w = 0;}
	Matrix(int h, int w);

	int h() const {return _h;}
	int w() const {return _w;}
	double &m(int n) {return _m[n];}
	double &m(int i, int j) {return _m[_w * i + j];}
	double const &m(int i, int j) const {return _m.at(_w * i + j);}

	bool isNull() const {return (_h == 0 || _w == 0);}

	bool eliminate(double epsilon = DBL_EPSILON);
	Matrix augemented(const Matrix &M) const;

private:
	QVector<double> _m;
	int _h;
	int _w;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Matrix &matrix);
#endif // QT_NO_DEBUG

#endif // MATRIX_H
