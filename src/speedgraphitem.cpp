#include "tooltip.h"
#include "speedgraphitem.h"

SpeedGraphItem::SpeedGraphItem(const Graph &graph, GraphType type,
  qreal movingTime, QGraphicsItem *parent) : GraphItem(graph, type, parent)
{
	_units = Metric;
	_timeType = Total;

	_avg = graph.last().s() / graph.last().t();
	_mavg = graph.last().s() / movingTime;

	setToolTip(toolTip());
}

QString SpeedGraphItem::toolTip() const
{
	ToolTip tt;
	qreal scale = (_units == Metric) ? MS2KMH : MS2MIH;
	QString su = (_units == Metric) ? tr("km/h") : tr("mi/h");

	tt.insert(tr("Maximum"), QString::number(max() * scale, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Average"), QString::number((_timeType == Total)
	  ? avg() * scale : mavg() * scale, 'f', 1) + UNIT_SPACE + su);

	return tt.toString();
}

void SpeedGraphItem::setUnits(Units units)
{
	_units = units;
	setToolTip(toolTip());
}

void SpeedGraphItem::setTimeType(TimeType type)
{
	_timeType = type;
	setToolTip(toolTip());
}
