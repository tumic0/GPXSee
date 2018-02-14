#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QTransform>
#include <QList>
#include "common/coordinates.h"

struct ReferencePoint {
	QPoint xy;
	QPointF pp;
};

class Transform
{
public:
	Transform(const QList<ReferencePoint> &points);

	bool isNull() {return _transform.type() == QTransform::TxNone;}
	const QString &errorString() const {return _errorString;}
	const QTransform &transform() const {return _transform;}

private:
	void simple(const QList<ReferencePoint> &points);
	void affine(const QList<ReferencePoint> &points);

	QTransform _transform;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const ReferencePoint &p);
#endif // QT_NO_DEBUG

#endif // TRANSFORM_H
