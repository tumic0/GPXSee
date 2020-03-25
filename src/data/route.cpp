#include "dem.h"
#include "route.h"

bool Route::_useDEM = false;
bool Route::_show2ndElevation = false;

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

Graph Route::gpsElevation() const
{
	Graph graph;
	graph.append(GraphSegment());
	GraphSegment &gs = graph.last();

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			gs.append(GraphPoint(_distance.at(i), NAN, _data.at(i).elevation()));

	return graph;
}

Graph Route::demElevation() const
{
	Graph graph;
	graph.append(GraphSegment());
	GraphSegment &gs = graph.last();

	for (int i = 0; i < _data.size(); i++) {
		qreal dem = DEM::elevation(_data.at(i).coordinates());
		if (!std::isnan(dem))
			gs.append(GraphPoint(_distance.at(i), NAN, dem));
	}

	return graph;
}

GraphPair Route::elevation() const
{
	if (_useDEM) {
		Graph dem(demElevation());
		if (dem.isValid())
			return GraphPair(dem, _show2ndElevation ? gpsElevation() : Graph());
		else
			return GraphPair(gpsElevation(), Graph());
	} else {
		Graph gps(gpsElevation());
		if (gps.isValid())
			return GraphPair(gps, _show2ndElevation ? demElevation() : Graph());
		else
			return GraphPair(demElevation(), Graph());
	}
}

qreal Route::distance() const
{
	return (_distance.isEmpty()) ? 0 : _distance.last();
}
