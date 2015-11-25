#include <QFile>
#include <QLineF>
#include "ll.h"
#include "gpx.h"


#define WINDOW_EF 3
#define WINDOW_SE 11
#define WINDOW_SF 11

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


bool GPX::loadFile(const QString &fileName)
{
	QFile file(fileName);
	bool ret;

	_data.clear();
	_error.clear();

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_error = qPrintable(file.errorString());
		return false;
	}

	if (!(ret = _parser.loadFile(&file, _data)))
		_error = _parser.errorString();
	file.close();

	return ret;
}

void GPX::elevationGraph(QVector<QPointF> &graph) const
{
	qreal dist = 0;
	QVector<QPointF> raw;

	if (!_data.size())
		return;

	raw.append(QPointF(0, _data.at(0).elevation));
	for (int i = 1; i < _data.size(); i++) {
		dist += llDistance(_data.at(i).coordinates, _data.at(i-1).coordinates);
		raw.append(QPointF(dist,  _data.at(i).elevation
		  - _data.at(i).geoidheight));
	}

	graph = filter(raw, WINDOW_EF);
}

void GPX::speedGraph(QVector<QPointF> &graph) const
{
	qreal dist = 0, v, ds, dt;
	QVector<QPointF> raw;

	if (!_data.size())
		return;

	raw.append(QPointF(0, 0));
	for (int i = 1; i < _data.size(); i++) {
		ds = llDistance(_data.at(i).coordinates, _data.at(i-1).coordinates);
		dt = _data.at(i-1).timestamp.msecsTo(_data.at(i).timestamp) / 1000.0;
		dist += ds;

		if (_data.at(i).speed < 0) {
			if (dt == 0)
				continue;
			v = ds / dt;
		} else
			v = _data.at(i).speed;

		raw.append(QPointF(dist, v));
	}

	graph = filter(eliminate(raw, WINDOW_SE), WINDOW_SF);
}

void GPX::track(QVector<QPointF> &track) const
{
	for (int i = 0; i < _data.size(); i++)
		track.append(ll2mercator(_data.at(i).coordinates));
}

qreal GPX::distance() const
{
	qreal dist = 0;

	for (int i = 1; i < _data.size(); i++)
		dist += llDistance(_data.at(i).coordinates, _data.at(i-1).coordinates);

	return dist;
}

qreal GPX::time() const
{
	if (_data.size() < 2)
		return 0;

	return (_data.at(0).timestamp.msecsTo(_data.at(_data.size() - 1).timestamp)
	  / 1000.0);
}

QDateTime GPX::date() const
{
	if (_data.size())
		return _data.at(0).timestamp;
	else
		return QDateTime();
}
