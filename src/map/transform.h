#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QTransform>
#include <QList>
#include <QDebug>
#include "pointd.h"

class ReferencePoint
{
public:
	ReferencePoint() {}
	ReferencePoint(const PointD &xy, const PointD &pp) : _xy(xy), _pp(pp) {}

	const PointD &xy() const {return _xy;}
	const PointD &pp() const {return _pp;}
	void setXY(const PointD &xy) {_xy = xy;}
	void setPP(const PointD &pp) {_pp = pp;}

private:
	PointD _xy, _pp;
};

class Transform
{
public:
	Transform();
	Transform(const QList<ReferencePoint> &points);
	Transform(const ReferencePoint &p1, const ReferencePoint &p2);
	Transform(const ReferencePoint &p, const PointD &scale);
	Transform(double matrix[16]);

	QPointF proj2img(const PointD &p) const
	  {return _proj2img.map(p.toPointF());}
	PointD img2proj(const QPointF &p) const
	  {return _img2proj.map(p);}

	bool isValid() const
	  {return _proj2img.isInvertible() && _img2proj.isInvertible();}
	const QString &errorString() const {return _errorString;}

private:
	void simple(const ReferencePoint &p1, const ReferencePoint &p2);
	void affine(const QList<ReferencePoint> &points);

	QTransform _proj2img, _img2proj;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const ReferencePoint &p);
#endif // QT_NO_DEBUG

#endif // TRANSFORM_H
