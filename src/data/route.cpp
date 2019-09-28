#include "route.h"


Route::Route(const RouteData &data) : _data(data)
{
	qreal dist = 0;

	_distance.append(0);

	for (int i = 1; i < _data.count(); i++) {
		dist += _data.at(i).coordinates().distanceTo(_data.at(i-1).coordinates());
		_distance.append(dist);
	}
}

Path Route::path() const
{
	Path ret;
	ret.append(PathSegment());
	PathSegment &ps = ret.last();

	for (int i = 0; i < _data.size(); i++)
		ps.append(PathPoint(_data.at(i).coordinates(), _distance.at(i)));

	return ret;
}

Graph Route::elevation() const
{
	Graph graph;
	graph.append(GraphSegment());
	GraphSegment &gs = graph.last();

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			gs.append(GraphPoint(_distance.at(i), NAN, _data.at(i).elevation()));

	return graph;
}

qreal Route::distance() const
{
	return (_distance.isEmpty()) ? 0 : _distance.last();
}
