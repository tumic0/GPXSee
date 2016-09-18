#include "ll.h"
#include "track.h"


#define WINDOW_EF 3
#define WINDOW_SE 11
#define WINDOW_SF 7
#define WINDOW_HE 11
#define WINDOW_HF 3

static bool lt(qreal v1, qreal v2)
{
	return v1 < v2;
}

static qreal median(QVector<qreal> v)
{
	qSort(v.begin(), v.end(), lt);
	return v.at(v.size() / 2);
}

static qreal MAD(QVector<qreal> v, qreal m)
{
	for (int i = 0; i < v.size(); i++)
		v[i] = (qAbs(v.at(i) - m));
	qSort(v.begin(), v.end(), lt);
	return v.at(v.size() / 2);
}

static QVector<qreal> eliminate(const QVector<qreal> &v, int window)
{
	QList<int> rm;
	QVector<qreal> ret;
	qreal m, M;


	if (v.size() < window)
		return QVector<qreal>(v);

	for (int i = window/2; i < v.size() - window/2; i++) {
		m = median(v.mid(i - window/2, window));
		M = MAD(v.mid(i - window/2, window), m);
		if (qAbs((0.6745 * (v.at(i) - m)) / M) > 3.5)
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

static QVector<qreal> filter(const QVector<qreal> &v, int window)
{
	qreal acc = 0;
	QVector<qreal> ret;

	if (v.size() < window)
		return QVector<qreal>(v);

	for (int i = 0; i < window; i++)
		acc += v.at(i);
	for (int i = 0; i <= window/2; i++)
		ret.append(acc/window);

	for (int i = window/2 + 1; i < v.size() - window/2; i++) {
		acc += v.at(i + window/2) - v.at(i - (window/2 + 1));
		ret.append(acc/window);
	}

	for (int i = v.size() - window/2; i < v.size(); i++)
		ret.append(acc/window);

	return ret;
}

Track::Track(const QVector<Trackpoint> &data) : _data(data)
{
	qreal dist = 0;
	qint64 time;

	_dd.append(0);
	_td.append(0);
	for (int i = 1; i < data.count(); i++) {
		dist += llDistance(data.at(i).coordinates(), data.at(i-1).coordinates());
		_dd.append(dist);
		if (data.first().hasTimestamp() && data.at(i).hasTimestamp()) {
			time = _data.first().timestamp().msecsTo(_data.at(i).timestamp());
			_td.append((qreal)time / 1000.0);
		}
	}

	if (_dd.size() != _td.size())
		_td.clear();
}

Graph Track::elevation() const
{
	Graph ret;
	QVector<qreal> raw;


	if (!_data.size())
		return ret;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			raw.append(_data.at(i).elevation() - _data.at(i).geoidHeight());

	ret.y = filter(raw, WINDOW_EF);
	ret.distance = _dd;
	ret.time = _td;

	return ret;
}

Graph Track::speed() const
{
	Graph ret;
	qreal v, ds, dt;
	QVector<qreal> raw;


	if (!_data.size())
		return ret;

	raw.append(0);
	for (int i = 1; i < _data.size(); i++) {
		if (_data.at(i).hasSpeed())
			v = _data.at(i).speed();
		else if (_data.at(i).hasTimestamp()) {
			dt = _td.at(i) - _td.at(i-1);
			if (!dt)
				continue;
			ds = _dd.at(i) - _dd.at(i-1);
			v = ds / dt;
		} else
			continue;

		raw.append(v);
	}

	ret.y = filter(eliminate(raw, WINDOW_SE), WINDOW_SF);
	ret.distance = _dd;
	ret.time = _td;

	return ret;
}

Graph Track::heartRate() const
{
	Graph ret;
	QVector<qreal> raw;

	if (!_data.size())
		return ret;

	for (int i = 0; i < _data.count(); i++)
		if (_data.at(i).hasHeartRate())
			raw.append(_data.at(i).heartRate());

	ret.y = filter(eliminate(raw, WINDOW_HE), WINDOW_HF);
	ret.distance = _dd;
	ret.time = _td;

	return ret;
}

Graph Track::temperature() const
{
	Graph ret;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasTemperature())
			ret.y.append(_data.at(i).temperature());

	ret.distance = _dd;
	ret.time = _td;

	return ret;
}

qreal Track::distance() const
{
	return (_dd.isEmpty()) ? 0 : _dd.last();
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
