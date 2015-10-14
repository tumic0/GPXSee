#include <QFile>
#include <QLineF>
#include "ll.h"
#include "gpx.h"


#define ALPHA_E 0.5
#define ALPHA_S 0.1

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
		acc = (i == 1) ? dh : (ALPHA_E * dh) + (1.0 - ALPHA_E) * acc;
		graph.append(QPointF(dist, acc));
	}
}

void GPX::speedGraph(QVector<QPointF> &graph) const
{
	qreal dist = 0, v, ds, dt, acc;

	if (!_data.size())
		return;

	graph.append(QPointF(0, 0));
	for (int i = 1; i < _data.size(); i++) {
		ds = llDistance(_data.at(i).coordinates, _data.at(i-1).coordinates);
		dt = _data.at(i-1).timestamp.msecsTo(_data.at(i).timestamp) / 1000.0;
		if (dt == 0)
			continue;
		dist += ds;
		v = ds / dt;
		acc = (i == 1) ? v : (ALPHA_S * v) + (1.0 - ALPHA_S) * acc;
		graph.append(QPointF(dist, acc));
	}
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
