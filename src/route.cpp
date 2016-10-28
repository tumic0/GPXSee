#include "route.h"

Route::Route(const RouteData &data) : _data(data)
{
	qreal dist = 0;

	_distance.append(dist);
	for (int i = 1; i < data.count(); i++) {
		dist += data.at(i).coordinates().distanceTo(data.at(i-1).coordinates());
		_distance.append(dist);
	}
}

Graph Route::elevation() const
{
	Graph graph;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			graph.append(GraphPoint(_distance.at(i), NAN,
			  _data.at(i).elevation()));

	return graph;
}

qreal Route::distance() const
{
	return (_distance.isEmpty()) ? 0 : _distance.last();
}
