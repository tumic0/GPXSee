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

	bool isValid() const {return size() >= 0;}

	void setMin(int min) {_min = min;}
	void setMax(int max) {_max = max;}

private:
	int _min, _max;
};

class RangeF
{
public:
	RangeF() {_min = 0; _max = 0;}
	RangeF(qreal min, qreal max) {_min = min, _max = max;}

	RangeF operator&(const RangeF &r) const;
	RangeF &operator&=(const RangeF &r) {*this = *this & r; return *this;}

	qreal min() const {return _min;}
	qreal max() const {return _max;}
	qreal size() const {return (_max - _min);}

	bool isNull() const {return _min == 0 && _max == 0;}
	bool isValid() const {return size() >= 0;}

	void setMin(qreal min) {_min = min;}
	void setMax(qreal max) {_max = max;}

	void resize(qreal size);

private:
	qreal _min, _max;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Range &range);
QDebug operator<<(QDebug dbg, const RangeF &range);
#endif // QT_NO_DEBUG

#endif // RANGE_H
