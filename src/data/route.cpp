#include "dem.h"
#include "route.h"


bool Route::_useDEMElevation = false;

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

	for (int i = 0; i < _data.size(); i++)
		ret.append(PathPoint(_data.at(i).coordinates(), _distance.at(i)));

	return ret;
}

Graph Route::elevation() const
{
	Graph graph;

	for (int i = 0; i < _data.size(); i++) {
		if (_data.at(i).hasElevation() && !_useDEMElevation)
			graph.append(GraphPoint(_distance.at(i), NAN,
			  _data.at(i).elevation()));
		else {
			qreal elevation = DEM::elevation(_data.at(i).coordinates());
			if (!std::isnan(elevation))
				graph.append(GraphPoint(_distance.at(i), NAN, elevation));
			else if (_data.at(i).hasElevation())
				graph.append(GraphPoint(_distance.at(i), NAN,
				  _data.at(i).elevation()));
		}
	}

	return graph;
}

qreal Route::distance() const
{
	return (_distance.isEmpty()) ? 0 : _distance.last();
}
