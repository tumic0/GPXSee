#include <QLocale>
#include "tooltip.h"
#include "format.h"
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
	qreal scale = (_units == Imperial) ? MS2MIH : (_units == Nautical)
	  ? MS2KN : MS2KMH;
	QString su = (_units == Imperial) ? tr("mi/h") : (_units == Nautical)
	  ? tr("kn") : tr("km/h");
	QString pace = Format::timeSpan((3600.0 / ((_timeType == Total)
	  ? avg() * scale : mavg() * scale)), false);
	QString pu = (_units == Metric) ? tr("min/km") : (_units == Imperial) ?
	  tr("min/mi") : tr("min/nmi");
	QLocale l(QLocale::system());

	tt.insert(tr("Maximum"), l.toString(max() * scale, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Average"), l.toString((_timeType == Total)
	  ? avg() * scale : mavg() * scale, 'f', 1) + UNIT_SPACE + su);
	tt.insert(tr("Pace"), pace + UNIT_SPACE + pu);

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
