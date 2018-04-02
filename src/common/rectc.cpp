#include "rectc.h"

RectC RectC::operator|(const RectC &r) const
{
	if (isNull())
		return r;
	if (r.isNull())
		return *this;

	qreal l1 = _tl.lon();
	qreal r1 = _tl.lon();
	if (_br.lon() - _tl.lon() < 0)
		l1 = _br.lon();
	else
		r1 = _br.lon();

	qreal l2 = r._tl.lon();
	qreal r2 = r._tl.lon();
	if (r._br.lon() - r._tl.lon() < 0)
		l2 = r._br.lon();
	else
		r2 = r._br.lon();

	qreal t1 = _tl.lat();
	qreal b1 = _tl.lat();
	if (_br.lat() - _tl.lat() < 0)
		t1 = _br.lat();
	else
		b1 = _br.lat();

	qreal t2 = r._tl.lat();
	qreal b2 = r._tl.lat();
	if (r._br.lat() - r._tl.lat() < 0)
		t2 = r._br.lat();
	else
		b2 = r._br.lat();

	RectC tmp;
	tmp._tl.setLon(qMin(l1, l2));
	tmp._br.setLon(qMax(r1, r2));
	tmp._tl.setLat(qMin(t1, t2));
	tmp._br.setLat(qMax(b1, b2));

	return tmp;
}

RectC RectC::operator&(const RectC &r) const
{
	if (isNull() || r.isNull())
		return RectC();

	qreal l1 = _tl.lon();
	qreal r1 = _tl.lon();
	if (_br.lon() - _tl.lon() < 0)
		l1 = _br.lon();
	else
		r1 = _br.lon();

	qreal l2 = r._tl.lon();
	qreal r2 = r._tl.lon();
	if (r._br.lon() - r._tl.lon() < 0)
		l2 = r._br.lon();
	else
		r2 = r._br.lon();

	if (l1 > r2 || l2 > r1)
		return RectC();

	qreal t1 = _tl.lat();
	qreal b1 = _tl.lat();
	if (_br.lat() - _tl.lat() < 0)
		t1 = _br.lat();
	else
		b1 = _br.lat();

	qreal t2 = r._tl.lat();
	qreal b2 = r._tl.lat();
	if (r._br.lat() - r._tl.lat() < 0)
		t2 = r._br.lat();
	else
		b2 = r._br.lat();

	if (t1 > b2 || t2 > b1)
		return RectC();

	RectC tmp;
	tmp._tl.setLon(qMax(l1, l2));
	tmp._br.setLon(qMin(r1, r2));
	tmp._tl.setLat(qMax(t1, t2));
	tmp._br.setLat(qMin(b1, b2));

	return tmp;
}

RectC RectC::normalized() const
{
	RectC r;

	if (_br.lon() < _tl.lon()) {
		r._tl.setLon(_br.lon());
		r._br.setLon(_tl.lon());
	} else {
		r._tl.setLon(_tl.lon());
		r._br.setLon(_br.lon());
	}
	if (_br.lat() < _tl.lat()) {
		r._tl.setLat(_br.lat());
		r._br.setLat(_tl.lat());
	} else {
		r._tl.setLat(_tl.lat());
		r._br.setLat(_br.lat());
	}

	return r;
}

void RectC::unite(const Coordinates &c)
{
	if (isNull()) {
		_tl = c;
		_br = c;
	} else {
		if (c.lon() < _tl.lon())
			_tl.setLon(c.lon());
		if (c.lon() > _br.lon())
			_br.setLon(c.lon());
		if (c.lat() > _br.lat())
			_br.setLat(c.lat());
		if (c.lat() < _tl.lat())
			_tl.setLat(c.lat());
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const RectC &rect)
{
	dbg.nospace() << "RectC(" << rect.topLeft() << ", " << rect.size() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
