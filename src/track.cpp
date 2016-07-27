#include "ll.h"
#include "track.h"


#define WINDOW_EF 3
#define WINDOW_SE 11
#define WINDOW_SF 7
#define WINDOW_HE 11
#define WINDOW_HF 3

static bool lt(const QPointF &p1, const QPointF &p2)
{
	return p1.y() < p2.y();
}

static qreal median(QVector<QPointF> v)
{
	qSort(v.begin(), v.end(), lt);
	return v.at(v.size() / 2).y();
}

static qreal MAD(QVector<QPointF> v, qreal m)
{
	for (int i = 0; i < v.size(); i++)
		v[i].setY(qAbs(v.at(i).y() - m));
	qSort(v.begin(), v.end(), lt);
	return v.at(v.size() / 2).y();
}

static QVector<QPointF> eliminate(const QVector<QPointF> &v, int window)
{
	QList<int> rm;
	QVector<QPointF> ret;
	qreal m, M;


	if (v.size() < window)
		return QVector<QPointF>(v);

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

static QVector<QPointF> filter(const QVector<QPointF> &v, int window)
{
	qreal acc = 0;
	QVector<QPointF> ret;

	if (v.size() < window)
		return QVector<QPointF>(v);

	for (int i = 0; i < window; i++)
		acc += v.at(i).y();
	for (int i = 0; i <= window/2; i++)
		ret.append(QPointF(v.at(i).x(), acc/window));

	for (int i = window/2 + 1; i < v.size() - window/2; i++) {
		acc += v.at(i + window/2).y() - v.at(i - (window/2 + 1)).y();
		ret.append(QPointF(v.at(i).x(), acc/window));
	}

	for (int i = v.size() - window/2; i < v.size(); i++)
		ret.append(QPointF(v.at(i).x(), acc/window));

	return ret;
}

Track::Track(const QVector<Trackpoint> &data) : _data(data)
{
	qreal dist = 0;

	_dd.append(dist);
	for (int i = 1; i < _data.count(); i++) {
		dist += llDistance(_data.at(i).coordinates(),
		  _data.at(i-1).coordinates());
		_dd.append(dist);
	}
}

void Track::elevationGraph(QVector<QPointF> &graph) const
{
	QVector<QPointF> raw;

	if (!_data.size())
		return;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			raw.append(QPointF(_dd.at(i), _data.at(i).elevation()
			  - _data.at(i).geoidHeight()));

	graph = filter(raw, WINDOW_EF);
}

void Track::speedGraph(QVector<QPointF> &graph) const
{
	qreal v, ds;
	qint64 dt;
	QVector<QPointF> raw;

	if (!_data.size())
		return;

	raw.append(QPointF(0, 0));
	for (int i = 1; i < _data.size(); i++) {
		if (_data.at(i).hasSpeed())
			v = _data.at(i).speed();
		else if (_data.at(i).hasTimestamp()) {
			dt = _data.at(i-1).timestamp().msecsTo(_data.at(i).timestamp());
			if (!dt)
				continue;
			ds = _dd.at(i) - _dd.at(i-1);
			v = ds / ((qreal)dt / 1000.0);
		} else
			continue;

		raw.append(QPointF(_dd.at(i), v));
	}

	graph = filter(eliminate(raw, WINDOW_SE), WINDOW_SF);
}

void Track::heartRateGraph(QVector<QPointF> &graph) const
{
	QVector<QPointF> raw;

	if (!_data.size())
		return;

	for (int i = 0; i < _data.count(); i++)
		if (_data.at(i).hasHeartRate())
			raw.append(QPointF(_dd.at(i), _data.at(i).heartRate()));

	graph = filter(eliminate(raw, WINDOW_HE), WINDOW_HF);
}

void Track::temperatureGraph(QVector<QPointF> &graph) const
{
	if (!_data.size())
		return;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasTemperature())
			graph.append(QPointF(_dd.at(i), _data.at(i).temperature()));
}

void Track::track(QVector<QPointF> &track) const
{
	for (int i = 0; i < _data.size(); i++)
		track.append(_data.at(i).coordinates());
}

qreal Track::time() const
{
	if (_data.size() < 2)
		return 0;

	return (_data.first().timestamp().msecsTo(_data.last().timestamp())
	  / 1000.0);
}

QDateTime Track::date() const
{
	if (_data.size())
		return _data.first().timestamp();
	else
		return QDateTime();
}
