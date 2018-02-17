#include "track.h"

#define OUTLIER_WINDOW 21

int Track::_elevationWindow = 3;
int Track::_speedWindow = 5;
int Track::_heartRateWindow = 3;
int Track::_cadenceWindow = 3;
int Track::_powerWindow = 3;

qreal Track::_pauseSpeed = 0.5;
int Track::_pauseInterval = 10;

bool Track::_outlierEliminate = true;


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
	qreal dt, ds, total;
	int last;


	_time.append(0);
	_distance.append(0);
	_speed.append(0);

	last = 0;

	for (int i = 1; i < _data.count(); i++) {
		ds = _data.at(i).coordinates().distanceTo(_data.at(i-1).coordinates());
		_distance.append(ds);

		if (_data.first().hasTimestamp() && _data.at(i).hasTimestamp()
		  && _data.at(i).timestamp() > _data.at(last).timestamp()) {
			_time.append(_data.first().timestamp().msecsTo(
			  _data.at(i).timestamp()) / 1000.0);
			last = i;
		} else
			_time.append(NAN);

		if (std::isnan(_time.at(i)) || std::isnan(_time.at(i-1)))
			_speed.append(NAN);
		else {
			dt = _time.at(i) - _time.at(i-1);
			if (dt < 1e-3) {
				_speed.append(_speed.at(i-1));
				continue;
			}
			_speed.append(ds / dt);
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

	if (_outlierEliminate)
		_outliers = eliminate(_speed, OUTLIER_WINDOW);

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

	return filter(raw, _elevationWindow);
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

	filtered = filter(raw, _speedWindow);

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

	filtered = filter(raw, _cadenceWindow);

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

	filtered = filter(raw, _powerWindow);

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
