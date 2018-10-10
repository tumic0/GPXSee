#include "track.h"

int Track::_elevationWindow = 3;
int Track::_speedWindow = 5;
int Track::_heartRateWindow = 3;
int Track::_cadenceWindow = 3;
int Track::_powerWindow = 3;

qreal Track::_pauseSpeed = 0.5;
int Track::_pauseInterval = 10;

bool Track::_outlierEliminate = true;
bool Track::_useReportedSpeed = false;


static qreal median(QVector<qreal> &v)
{
	qSort(v.begin(), v.end());
	return v.at(v.size() / 2);
}

static qreal MAD(QVector<qreal> &v, qreal m)
{
	for (int i = 0; i < v.size(); i++)
		v[i] = qAbs(v.at(i) - m);
	qSort(v.begin(), v.end());
	return v.at(v.size() / 2);
}

static QSet<int> eliminate(const QVector<qreal> &v)
{
	QSet<int> rm;

	QVector<qreal> w(v);
	qreal m = median(w);
	qreal M = MAD(w, m);

	for (int i = 0; i < v.size(); i++)
		if (qAbs((0.6745 * (v.at(i) - m)) / M) > 5)
			rm.insert(i);

	return rm;
}

static Graph filter(const Graph &g, int window)
{
	if (g.size() < window || window < 2)
		return Graph(g);

	qreal acc = 0;
	Graph ret(g.size());

	for (int i = 0; i < window; i++)
		acc += g.at(i).y();
	for (int i = 0; i <= window/2; i++)
		ret[i] = GraphPoint(g.at(i).s(), g.at(i).t(), acc/window);

	for (int i = window/2 + 1; i < g.size() - window/2; i++) {
		acc += g.at(i + window/2).y() - g.at(i - (window/2 + 1)).y();
		ret[i] = GraphPoint(g.at(i).s(), g.at(i).t(), acc/window);
	}

	for (int i = g.size() - window/2; i < g.size(); i++)
		ret[i] = GraphPoint(g.at(i).s(), g.at(i).t(), acc/window);

	return ret;
}


Track::Track(const TrackData &data) : _data(data)
{
	QVector<qreal> acceleration;
	qreal ds, dt;

	_time.append(0);
	_distance.append(0);
	_speed.append(0);
	acceleration.append(0);

	for (int i = 1; i < _data.count(); i++) {
		ds = _data.at(i).coordinates().distanceTo(_data.at(i-1).coordinates());
		_distance.append(_distance.at(i-1) + ds);

		if (_data.first().hasTimestamp() && _data.at(i).hasTimestamp()
		  && _data.at(i).timestamp() >= _data.at(i-1).timestamp())
			_time.append(_data.first().timestamp().msecsTo(
			  _data.at(i).timestamp()) / 1000.0);
		else
			_time.append(NAN);

		dt = _time.at(i) - _time.at(i-1);
		if (dt < 1e-3) {
			_speed.append(_speed.at(i-1));
			acceleration.append(acceleration.at(i-1));
		} else {
			_speed.append(ds / dt);
			qreal dv = _speed.at(i) - _speed.at(i-1);
			acceleration.append(dv / dt);
		}
	}

	_pause = 0;
	for (int i = 1; i < _data.count(); i++) {
		if (_time.at(i) > _time.at(i-1) + _pauseInterval
		  && _speed.at(i) < _pauseSpeed) {
			_pause += _time.at(i) - _time.at(i-1);
			_stop.insert(i-1);
			_stop.insert(i);
		}
	}

	if (!_outlierEliminate)
		return;

	_outliers = eliminate(acceleration);

	QSet<int>::const_iterator it;
	for (it = _stop.constBegin(); it != _stop.constEnd(); ++it)
		_outliers.remove(*it);

	int last = 0;
	for (int i = 0; i < _data.size(); i++) {
		if (_outliers.contains(i))
			last++;
		else
			break;
	}
	for (int i = last + 1; i < _data.size(); i++) {
		if (_outliers.contains(i))
			continue;
		if (discardStopPoint(i)) {
			_distance[i] = _distance.at(last);
			_speed[i] = 0;
		} else {
			ds = _data.at(i).coordinates().distanceTo(
			  _data.at(last).coordinates());
			_distance[i] = _distance.at(last) + ds;

			dt = _time.at(i) - _time.at(last);
			_speed[i] = (dt < 1e-3) ? _speed.at(last) : ds / dt;
		}
		last = i;
	}
}

Graph Track::elevation() const
{
	Graph raw;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation() && !_outliers.contains(i))
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).elevation()));

	return filter(raw, _elevationWindow);
}

Graph Track::speed() const
{
	Graph raw, filtered;
	qreal v;
	QList<int> stop;

	for (int i = 0; i < _data.size(); i++) {
		if (_stop.contains(i) && (!std::isnan(_speed.at(i))
		  || _data.at(i).hasSpeed())) {
			v = 0;
			stop.append(raw.size());
		} else if (_useReportedSpeed && _data.at(i).hasSpeed()
		  && !_outliers.contains(i))
			v = _data.at(i).speed();
		else if (!std::isnan(_speed.at(i)) && !_outliers.contains(i))
			v = _speed.at(i);
		else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), v));
	}

	filtered = filter(raw, _speedWindow);

	for (int i = 0; i < stop.size(); i++)
		filtered[stop.at(i)].setY(0);

	return filtered;
}

Graph Track::heartRate() const
{
	Graph raw;

	for (int i = 0; i < _data.count(); i++)
		if (_data.at(i).hasHeartRate() && !_outliers.contains(i))
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).heartRate()));

	return filter(raw, _heartRateWindow);
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

Graph Track::ratio() const
{
	Graph raw;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasRatio() && !_outliers.contains(i))
			raw.append(GraphPoint(_distance.at(i), _time.at(i),
			  _data.at(i).ratio()));

	return raw;
}

Graph Track::cadence() const
{
	Graph raw, filtered;
	QList<int> stop;
	qreal c;

	for (int i = 0; i < _data.size(); i++) {
		if (_data.at(i).hasCadence() && _stop.contains(i)) {
			c = 0;
			stop.append(raw.size());
		} else if (_data.at(i).hasCadence() && !_outliers.contains(i))
			c = _data.at(i).cadence();
		else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), c));
	}

	filtered = filter(raw, _cadenceWindow);

	for (int i = 0; i < stop.size(); i++)
		filtered[stop.at(i)].setY(0);

	return filtered;
}

Graph Track::power() const
{
	Graph raw, filtered;
	QList<int> stop;
	qreal p;

	for (int i = 0; i < _data.size(); i++) {
		if (_data.at(i).hasPower() && _stop.contains(i)) {
			p = 0;
			stop.append(raw.size());
		} else if (_data.at(i).hasPower() && !_outliers.contains(i))
			p = _data.at(i).power();
		else
			continue;

		raw.append(GraphPoint(_distance.at(i), _time.at(i), p));
	}

	filtered = filter(raw, _powerWindow);

	for (int i = 0; i < stop.size(); i++)
		filtered[stop.at(i)].setY(0);

	return filtered;
}

qreal Track::distance() const
{
	for (int i = _distance.size() - 1; i >= 0; i--)
		if (!_outliers.contains(i))
			return _distance.at(i);

	return 0;
}

qreal Track::time() const
{
	for (int i = _data.size() - 1; i >= 0; i--)
		if (!_outliers.contains(i))
			return _data.first().timestamp().msecsTo(_data.at(i).timestamp())
			  / 1000.0;

	return 0;
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
