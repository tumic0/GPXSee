#include "tooltip.h"
#include "speedgraphitem.h"

SpeedGraphItem::SpeedGraphItem(const Graph &graph, qreal movingTime,
  QGraphicsItem *parent) : GraphItem(graph, parent)
{
	_avg = graph.last().s() / graph.last().t();
	_mavg = graph.last().s() / movingTime;

	setToolTip(toolTip(Metric));
}

QString SpeedGraphItem::toolTip(Units units) const
{
	ToolTip tt;
	qreal scale = (units == Metric) ? MS2KMH : MS2MIH;
	QString su = (units == Metric) ? tr("km/h") : tr("mi/h");

	tt.insert(tr("Maximum"), QString::number(max() * scale, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Average"), QString::number(avg() * scale, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Moving average"), QString::number(mavg() * scale, 'f', 1)
	  + UNIT_SPACE + su);

	return tt.toString();
}

void SpeedGraphItem::setUnits(Units units)
{
	setToolTip(toolTip(units));
}
