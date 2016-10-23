#include "track.h"


#define WINDOW_EF 3
#define WINDOW_SE 11
#define WINDOW_SF 7
#define WINDOW_HE 11
#define WINDOW_HF 3


static bool lt(const GraphPoint &v1, const GraphPoint &v2)
{
	return v1.y() < v2.y();
}

static qreal median(QVector<GraphPoint> v)
{
	qSort(v.begin(), v.end(), lt);
	return v.at(v.size() / 2).y();
}

static qreal MAD(QVector<GraphPoint> v, qreal m)
{
	for (int i = 0; i < v.size(); i++)
		v[i].setY(qAbs(v.at(i).y() - m));
	qSort(v.begin(), v.end(), lt);
	return v.at(v.size() / 2).y();
}

static QVector<GraphPoint> eliminate(const QVector<GraphPoint> &v, int window)
{
	QList<int> rm;
	QVector<GraphPoint> ret;
	qreal m, M;


	if (v.size() < window)
		return QVector<GraphPoint>(v);

	for (int i = window/2; i < v.size() - window/2; i++) {
		m = median(v.mid(i - window/2, window));
		M = MAD(v.mid(i - window/2, window), m);
		if (qAbs((0.6745 * (v.at(i).y() - m)) / M) > 3.5)
			rm.append(i);
	}

	QList<int>::const_iterator it = rm.begin();
	for (int i = 0; i < v.size(); i++) {
		if (it == rm.end() || *it != i)
			ret.append(v.at(i));
		else
			it++;
	}

	return ret;
}

static QVector<GraphPoint> filter(const QVector<GraphPoint> &v, int window)
{
	qreal acc = 0;
	QVector<GraphPoint> ret;

	if (v.size() < window)
		return QVector<GraphPoint>(v);

	for (int i = 0; i < window; i++)
		acc += v.at(i).y();
	for (int i = 0; i <= window/2; i++)
		ret.append(GraphPoint(v.at(i).s(), v.at(i).t(), acc/window));

	for (int i = window/2 + 1; i < v.size() - window/2; i++) {
		acc += v.at(i + window/2).y() - v.at(i - (window/2 + 1)).y();
		ret.append(GraphPoint(v.at(i).s(), v.at(i).t(), acc/window));
	}

	for (int i = v.size() - window/2; i < v.size(); i++)
		ret.append(GraphPoint(v.at(i).s(), v.at(i).t(), acc/window));

	return ret;
}

Track::Track(const QVector<Trackpoint> &data) : _data(data)
{
	qreal dist = 0;

	_distance.append(0);
	_time.append(0);
	for (int i = 1; i < data.count(); i++) {
		dist += data.at(i).coordinates().distanceTo(data.at(i-1).coordinates());
		_distance.append(dist);

		if (data.first().hasTimestamp() && data.at(i).hasTimestamp())
			_time.append(_data.first().timestamp().msecsTo(
			  _data.at(i).timestamp()) / 1000.0);
		else
			_time.append(NAN);
	}
}

Graph Track::elevation() const
{
	QVector<GraphPoint> raw;

	if (!_data.size())
		return raw;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).elevation() - _data.at(i).geoidHeight()));

	return filter(raw, WINDOW_EF);
}

Graph Track::speed() const
{
	QVector<GraphPoint> raw;
	qreal v, ds, dt;

	if (!_data.size())
		return raw;

	raw.append(GraphPoint(_distance.at(0), _time.at(0), 0));
	for (int i = 1; i < _data.size(); i++) {
		if (_data.at(i).hasSpeed())
			v = _data.at(i).speed();
		else if (_data.at(i).hasTimestamp() && _data.at(i-1).hasTimestamp()) {
			dt = _time.at(i) - _time.at(i-1);
			if (!dt)
				continue;
			ds = _distance.at(i) - _distance.at(i-1);
			v = ds / dt;
		} else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), v));
	}

	return filter(eliminate(raw, WINDOW_SE), WINDOW_SF);
}

Graph Track::heartRate() const
{
	QVector<GraphPoint> raw;

	if (!_data.size())
		return raw;

	for (int i = 0; i < _data.count(); i++)
		if (_data.at(i).hasHeartRate())
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).heartRate()));

	return filter(eliminate(raw, WINDOW_HE), WINDOW_HF);
}

Graph Track::temperature() const
{
	QVector<GraphPoint> raw;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasTemperature())
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).temperature()));

	return Graph(raw);
}

qreal Track::distance() const
{
	return (_distance.isEmpty()) ? 0 : _distance.last();
}

qreal Track::time() const
{
	return (_data.size() < 2) ? 0 :
	  (_data.first().timestamp().msecsTo(_data.last().timestamp()) / 1000.0);
}

QDateTime Track::date() const
{
	return (_data.size()) ? _data.first().timestamp() : QDateTime();
}
