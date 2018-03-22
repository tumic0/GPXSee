#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QTransform>
#include <QList>
#include <QDebug>

struct ReferencePoint {
	QPoint xy;
	QPointF pp;
};

class Transform
{
public:
	Transform();
	Transform(const QList<ReferencePoint> &points);
	Transform(const ReferencePoint &p, const QPointF &scale);
	Transform(double m[16]);

	QPointF proj2img(const QPointF &p) const {return _proj2img.map(p);}
	QPointF img2proj(const QPointF &p) const {return _img2proj.map(p);}

	bool isValid() const
	  {return _proj2img.isInvertible() && _img2proj.isInvertible();}
	const QString &errorString() const {return _errorString;}

private:
	void simple(const QList<ReferencePoint> &points);
	void affine(const QList<ReferencePoint> &points);

	QTransform _proj2img, _img2proj;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const ReferencePoint &p);
#endif // QT_NO_DEBUG

#endif // TRANSFORM_H
