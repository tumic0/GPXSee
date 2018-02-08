#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QCoreApplication>
#include <QTransform>
#include <QList>
#include "common/coordinates.h"

struct ReferencePoint {
	QPoint xy;
	QPointF pp;
};

class Transform
{
	Q_DECLARE_TR_FUNCTIONS(Transform)

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

QDebug operator<<(QDebug dbg, const ReferencePoint &p);

#endif // TRANSFORM_H
