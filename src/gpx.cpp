#include <QFile>
#include <QLineF>
#include "ll.h"
#include "gpx.h"

#include <QDebug>


#define ALPHA  0.5
#define WINDOW 11


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
	qreal dist = 0, dh, acc;

	if (!_data.size())
		return;

	graph.append(QPointF(0, _data.at(0).elevation));
	for (int i = 1; i < _data.size(); i++) {
		dist += llDistance(_data.at(i).coordinates, _data.at(i-1).coordinates);
		dh = _data.at(i).elevation;
		acc = (i == 1) ? dh : (ALPHA * dh) + (1.0 - ALPHA) * acc;
		graph.append(QPointF(dist, acc));
	}
}

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

static QVector<QPointF> filter(const QVector<QPointF> &v)
{
	QList<int> rm;
	QVector<QPointF> ret;
	qreal m, M;


	if (v.size() < WINDOW)
		return QVector<QPointF>(v);

	for (int i = WINDOW/2; i < v.size() - WINDOW/2; i++) {
		m = median(v.mid(i - WINDOW/2, WINDOW));
		M = MAD(v.mid(i - WINDOW/2, WINDOW), m);
		if (qAbs((0.6745 * (v.at(i).y() - m)) / M) > 3.5)
			rm.append(i);
	}

	QList<int>::const_iterator it = rm.begin();
	for (int i = 0; i < v.size(); i++) {
		if (*it != i)
			ret.append(v.at(i));
		else
			it++;
	}

	return ret;
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

	graph = filter(raw);
}

void GPX::track(QVector<QPointF> &track) const
{
	QPointF p;

	for (int i = 0; i < _data.size(); i++) {
		ll2mercator(_data.at(i).coordinates, p);
		track.append(p);
	}
}

qreal GPX::distance()
{
	qreal dist = 0;

	for (int i = 1; i < _data.size(); i++)
		dist += llDistance(_data.at(i).coordinates, _data.at(i-1).coordinates);

	return dist;
}

qreal GPX::time()
{
	if (_data.size() < 2)
		return 0;

	return (_data.at(0).timestamp.msecsTo(_data.at(_data.size() - 1).timestamp)
	  / 1000.0);
}
