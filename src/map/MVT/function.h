#ifndef MVT_FUNCTION_H
#define MVT_FUNCTION_H

#include <QList>
#include <QPointF>
#include <QColor>
#include <QPair>
#include <QString>
#include <QJsonValue>

namespace MVT {

class FunctionB {
public:
	FunctionB(bool deflt = true) : _default(deflt) {}
	FunctionB(const QJsonValue &json, bool dflt = true);

	bool value(qreal x) const;

private:
	QList<QPair<qreal, bool> > _stops;
	bool _default;
};

class FunctionS {
public:
	FunctionS(const QString &deflt = QString()) : _default(deflt) {}
	FunctionS(const QJsonValue &json, const QString &dflt = QString());

	const QString value(qreal x) const;

private:
	QList<QPair<qreal, QString> > _stops;
	QString _default;
};

class FunctionF {
public:
	FunctionF(qreal deflt = 0) : _default(deflt), _base(1.0) {}
	FunctionF(const QJsonValue &json, qreal dflt = 0);

	qreal value(qreal x) const;

private:
	QList<QPointF> _stops;
	qreal _default;
	qreal _base;
};

class FunctionC {
public:
	FunctionC(const QColor &deflt = QColor(Qt::black))
	  : _default(deflt), _base(1.0) {}
	FunctionC(const QJsonValue &json, const QColor &dflt = QColor(Qt::black));

	QColor value(qreal x) const;

private:
	QList<QPair<qreal, QColor> > _stops;
	QColor _default;
	qreal _base;
};

}

#endif // MVT_FUNCTION_H
