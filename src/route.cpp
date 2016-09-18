#include "ll.h"
#include "route.h"

Route::Route(const QVector<Waypoint> &data) : _data(data)
{
	qreal dist = 0;

	_dd.append(dist);
	for (int i = 1; i < data.count(); i++) {
		dist += llDistance(data.at(i).coordinates(), data.at(i-1).coordinates());
		_dd.append(dist);
	}
}

Graph Route::elevation() const
{
	Graph graph;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			graph.y.append(_data.at(i).elevation() - _data.at(i).geoidHeight());

	graph.distance = _dd;

	return graph;
}

qreal Route::distance() const
{
	return (_dd.isEmpty()) ? 0 : _dd.last();
}
