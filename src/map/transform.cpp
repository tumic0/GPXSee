#include "projection.h"
#include "matrix.h"
#include "transform.h"


#define NULL_QTRANSFORM 0,0,0,0,0,0,0,0,0

void Transform::simple(const ReferencePoint &p1, const ReferencePoint &p2)
{
	if (p1.xy().x() == p2.xy().x() || p1.xy().y() == p2.xy().y()) {
		_errorString = "Invalid reference points tuple";
		return;
	}

	double sX = (p1.xy().x() - p2.xy().x()) / (p1.pp().x() - p2.pp().x());
	double sY = (p2.xy().y() - p1.xy().y()) / (p2.pp().y() - p1.pp().y());
	double dX = p2.xy().x() - p2.pp().x() * sX;
	double dY = p1.xy().y() - p1.pp().y() * sY;

	_proj2img = QTransform(sX, 0, 0, sY, dX, dY);
	_img2proj = _proj2img.inverted();
}

void Transform::affine(const QList<ReferencePoint> &points)
{
	Matrix c(3, 2);
	c.zeroize();
	for (size_t i = 0; i < c.h(); i++) {
		for (size_t j = 0; j < c.w(); j++) {
			for (int k = 0; k < points.size(); k++) {
				double f[3], t[2];

				f[0] = points.at(k).pp().x();
				f[1] = points.at(k).pp().y();
				f[2] = 1.0;
				t[0] = points.at(k).xy().x();
				t[1] = points.at(k).xy().y();
				c.m(i,j) += f[i] * t[j];
			}
		}
	}

	Matrix Q(3, 3);
	Q.zeroize();
	for (int qi = 0; qi < points.size(); qi++) {
		double v[3];

		v[0] = points.at(qi).pp().x();
		v[1] = points.at(qi).pp().y();
		v[2] = 1.0;
		for (size_t i = 0; i < Q.h(); i++)
			for (size_t j = 0; j < Q.w(); j++)
				Q.m(i,j) += v[i] * v[j];
	}

	Matrix M = Q.augemented(c);
	if (!M.eliminate()) {
		_errorString = "Singular transformation matrix";
		return;
	}

	_proj2img = QTransform(M.m(0,3), M.m(0,4), M.m(1,3), M.m(1,4), M.m(2,3),
	  M.m(2,4));
	_img2proj = _proj2img.inverted();
}


Transform::Transform()
  : _proj2img(NULL_QTRANSFORM), _img2proj(NULL_QTRANSFORM)
{
}

Transform::Transform(const QList<ReferencePoint> &points)
  : _proj2img(NULL_QTRANSFORM), _img2proj(NULL_QTRANSFORM)
{
	if (points.count() < 2)
		_errorString = "Insufficient number of reference points";
	else if (points.size() == 2)
		simple(points.at(0), points.at(1));
	else
		affine(points);
}

Transform::Transform(const ReferencePoint &p1, const ReferencePoint &p2)
  : _proj2img(NULL_QTRANSFORM), _img2proj(NULL_QTRANSFORM)
{
	simple(p1, p2);
}

Transform::Transform(const ReferencePoint &p, const PointD &scale)
  : _proj2img(NULL_QTRANSFORM), _img2proj(NULL_QTRANSFORM)
{
	if (scale.x() == 0.0 || scale.y() == 0.0) {
		_errorString = "Invalid scale factor";
		return;
	}

	_img2proj = QTransform(scale.x(), 0, 0, -scale.y(), p.pp().x() - p.xy().x()
	  / scale.x(), p.pp().y() + p.xy().x() / scale.y());
	_proj2img = _img2proj.inverted();
}

Transform::Transform(double matrix[16])
  : _proj2img(NULL_QTRANSFORM), _img2proj(NULL_QTRANSFORM)
{
	_img2proj = QTransform(matrix[0], matrix[1], matrix[4], matrix[5],
	  matrix[3], matrix[7]);
	if (!_img2proj.isInvertible())
		_errorString = "Singular transformation matrix";
	else
		_proj2img = _img2proj.inverted();
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const ReferencePoint &p)
{
	dbg.nospace() << "ReferencePoint(" << p.xy() << ", " << p.pp() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
