#include "track.h"


#define WINDOW_OE 31

#define WINDOW_EF 3
#define WINDOW_SF 7
#define WINDOW_HF 3
#define WINDOW_CF 3
#define WINDOW_PF 3

#define PAUSE_SPEED     0.5
#define PAUSE_TIME_DIFF 10


static qreal median(QVector<qreal> v)
{
	qSort(v.begin(), v.end());
	return v.at(v.size() / 2);
}

static qreal MAD(QVector<qreal> v, qreal m)
{
	for (int i = 0; i < v.size(); i++)
		v[i] = qAbs(v.at(i) - m);
	qSort(v.begin(), v.end());
	return v.at(v.size() / 2);
}

static QSet<int> eliminate(const QVector<qreal> &v, int window)
{
	QSet<int> rm;
	qreal m, M;


	if (v.size() < window)
		return rm;

	for (int i = window/2; i < v.size() - window/2; i++) {
		m = median(v.mid(i - window/2, window));
		M = MAD(v.mid(i - window/2, window), m);
		if (qAbs((0.6745 * (v.at(i) - m)) / M) > 3.5)
			rm.insert(i);
	}

	return rm;
}

static Graph filter(const Graph &g, int window)
{
	qreal acc = 0;
	Graph ret;

	if (g.size() < window)
		return ret;

	for (int i = 0; i < window; i++)
		acc += g.at(i).y();
	for (int i = 0; i <= window/2; i++)
		ret.append(GraphPoint(g.at(i).s(), g.at(i).t(), acc/window));

	for (int i = window/2 + 1; i < g.size() - window/2; i++) {
		acc += g.at(i + window/2).y() - g.at(i - (window/2 + 1)).y();
		ret.append(GraphPoint(g.at(i).s(), g.at(i).t(), acc/window));
	}

	for (int i = g.size() - window/2; i < g.size(); i++)
		ret.append(GraphPoint(g.at(i).s(), g.at(i).t(), acc/window));

	return ret;
}


Track::Track(const TrackData &data) : _data(data)
{
	qreal dt, ds, total;


	_time.append(0);
	_distance.append(0);
	_speed.append(0);

	for (int i = 1; i < data.count(); i++) {
		ds = data.at(i).coordinates().distanceTo(data.at(i-1).coordinates());
		_distance.append(ds);

		if (data.first().hasTimestamp() && data.at(i).hasTimestamp())
			_time.append(_data.first().timestamp().msecsTo(
			  _data.at(i).timestamp()) / 1000.0);
		else
			_time.append(NAN);

		if (std::isnan(_time.at(i)) || std::isnan(_time.at(i-1)))
			_speed.append(NAN);
		else {
			dt = _time.at(i) - _time.at(i-1);
			if (!dt) {
				_speed.append(_speed.at(i-1));
				continue;
			}
			_speed.append(ds / dt);
		}
	}

	_pause = 0;
	for (int i = 1; i < data.count(); i++) {
		if (_time.at(i) > _time.at(i-1) + PAUSE_TIME_DIFF
		  && _speed.at(i) < PAUSE_SPEED) {
			_pause += _time.at(i) - _time.at(i-1);
			_stop.insert(i-1);
			_stop.insert(i);
		}
	}

	_outliers = eliminate(_speed, WINDOW_OE);

	QSet<int>::const_iterator it;
	for (it = _stop.constBegin(); it != _stop.constEnd(); ++it)
		_outliers.remove(*it);

	total = 0;
	for (int i = 0; i < _data.size(); i++) {
		if (_outliers.contains(i))
			continue;
		if (!discardStopPoint(i))
			total += _distance.at(i);
		_distance[i] = total;
	}
}

Graph Track::elevation() const
{
	Graph raw;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation() && !_outliers.contains(i))
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).elevation()));

	return filter(raw, WINDOW_EF);
}

Graph Track::speed() const
{
	Graph raw, filtered;
	qreal v;
	QSet<int> stop;

	for (int i = 0; i < _data.size(); i++) {
		if (_stop.contains(i) && (!std::isnan(_speed.at(i))
		  || _data.at(i).hasSpeed())) {
			v = 0;
			stop.insert(raw.size());
		} else if (_data.at(i).hasSpeed() && !_outliers.contains(i))
			v = _data.at(i).speed();
		else if (!std::isnan(_speed.at(i)) && !_outliers.contains(i))
			v = _speed.at(i);
		else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), v));
	}

	filtered = filter(raw, WINDOW_SF);

	QSet<int>::const_iterator it;
	for (it = stop.constBegin(); it != stop.constEnd(); ++it)
		filtered[*it].setY(0);

	return filtered;
}

Graph Track::heartRate() const
{
	Graph raw;

	for (int i = 0; i < _data.count(); i++)
		if (_data.at(i).hasHeartRate() && !_outliers.contains(i))
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).heartRate()));

	return filter(raw, WINDOW_HF);
}

Graph Track::temperature() const
{
	Graph raw;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasTemperature() && !_outliers.contains(i))
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).temperature()));

	return raw;
}

Graph Track::cadence() const
{
	Graph raw, filtered;
	QSet<int> stop;
	qreal c;

	for (int i = 0; i < _data.size(); i++) {
		if (_data.at(i).hasCadence() && _stop.contains(i)) {
			c = 0;
			stop.insert(raw.size());
		} else if (_data.at(i).hasCadence() && !_outliers.contains(i))
			c = _data.at(i).cadence();
		else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), c));
	}

	filtered = filter(raw, WINDOW_CF);

	QSet<int>::const_iterator it;
	for (it = stop.constBegin(); it != stop.constEnd(); ++it)
		filtered[*it].setY(0);

	return filtered;
}

Graph Track::power() const
{
	Graph raw, filtered;
	QSet<int> stop;
	qreal p;

	for (int i = 0; i < _data.size(); i++) {
		if (_data.at(i).hasPower() && _stop.contains(i)) {
			p = 0;
			stop.insert(raw.size());
		} else if (_data.at(i).hasPower() && !_outliers.contains(i))
			p = _data.at(i).power();
		else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), p));
	}

	filtered = filter(raw, WINDOW_PF);

	QSet<int>::const_iterator it;
	for (it = stop.constBegin(); it != stop.constEnd(); ++it)
		filtered[*it].setY(0);

	return filtered;
}

qreal Track::distance() const
{
	return _distance.isEmpty() ? 0 : _distance.last();
}

qreal Track::time() const
{
	return (_data.size() < 2) ? 0 :
	  (_data.first().timestamp().msecsTo(_data.last().timestamp()) / 1000.0);
}

qreal Track::movingTime() const
{
	return (time() - _pause);
}

QDateTime Track::date() const
{
	return (_data.size()) ? _data.first().timestamp() : QDateTime();
}

Path Track::path() const
{
	Path ret;

	for (int i = 0; i < _data.size(); i++)
		if (!_outliers.contains(i) && !discardStopPoint(i))
			ret.append(PathPoint(_data.at(i).coordinates(), _distance.at(i)));

	return ret;
}

bool Track::discardStopPoint(int i) const
{
	return (_stop.contains(i) && i > 0 && _stop.contains(i-1)
	  && i < _data.size() - 1 && _stop.contains(i+1));
}
