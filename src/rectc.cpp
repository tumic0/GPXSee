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

void RectC::unite(const Coordinates &c)
{
	if (c.lon() < _tl.lon())
		_tl.setLon(c.lon());
	if (c.lon() > _br.lon())
		_br.setLon(c.lon());
	if (c.lat() > _br.lat())
		_br.setLat(c.lat());
	if (c.lat() < _tl.lat())
		_tl.setLat(c.lat());
}

QDebug operator<<(QDebug dbg, const RectC &rect)
{
	dbg.nospace() << "RectC(" << rect.topLeft() << ", " << rect.size() << ")";
	return dbg.maybeSpace();
}
