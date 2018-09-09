#include <QLocale>
#include "tooltip.h"
#include "temperaturegraphitem.h"


TemperatureGraphItem::TemperatureGraphItem(const Graph &graph, GraphType type,
  QGraphicsItem *parent) : GraphItem(graph, type, parent)
{
	qreal sum = 0;

	for (int j = 1; j < graph.size(); j++)
		sum += graph.at(j).y() * (graph.at(j).s() - graph.at(j-1).s());
	_avg = sum/graph.last().s();

	setToolTip(toolTip(Metric));
}

QString TemperatureGraphItem::toolTip(Units units) const
{
	ToolTip tt;
	qreal scale = (units == Metric) ? 1.0 : C2FS;
	qreal offset = (units == Metric) ? 0 : C2FO;
	QString su = (units == Metric) ?
	  QChar(0x00B0) + tr("C") : QChar(0x00B0) + tr("F");
	QLocale l(QLocale::system());

	tt.insert(tr("Average"), l.toString(avg() * scale + offset, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Maximum"), l.toString(max() * scale + offset, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Minimum"), l.toString(min() * scale + offset, 'f', 1)
	  + UNIT_SPACE + su);

	return tt.toString();
}

void TemperatureGraphItem::setUnits(Units units)
{
	setToolTip(toolTip(units));
}
