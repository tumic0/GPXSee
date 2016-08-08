#include "ll.h"
#include "route.h"

Route::Route(const QVector<Waypoint> &data) : _data(data)
{
	qreal dist = 0;

	_dd.append(dist);
	for (int i = 1; i < _data.count(); i++) {
		dist += llDistance(_data.at(i).coordinates(),
		  _data.at(i-1).coordinates());
		_dd.append(dist);
	}
}

void Route::elevationGraph(QVector<QPointF> &graph) const
{
	if (!_data.size())
		return;

	for (int i = 0; i < _data.size(); i++)
		if (_data.at(i).hasElevation())
			graph.append(QPointF(_dd.at(i), _data.at(i).elevation()
			  - _data.at(i).geoidHeight()));
}
