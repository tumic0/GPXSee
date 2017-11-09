#ifndef RANGE_H
#define RANGE_H

#include <QtGlobal>
#include <QDebug>

class Range
{
public:
	Range() {_min = 0; _max = 0;}
	Range(int min, int max) {_min = min, _max = max;}

	int min() const {return _min;}
	int max() const {return _max;}
	int size() const {return (_max - _min);}

private:
	int _min, _max;
};

class RangeF
{
public:
	RangeF() {_min = 0; _max = 0;}
	RangeF(qreal min, qreal max) {_min = min, _max = max;}

	qreal min() const {return _min;}
	qreal max() const {return _max;}
	qreal size() const {return (_max - _min);}

	void resize(qreal size);

private:
	qreal _min, _max;
};

QDebug operator<<(QDebug dbg, const Range &range);
QDebug operator<<(QDebug dbg, const RangeF &range);

#endif // RANGE_H
