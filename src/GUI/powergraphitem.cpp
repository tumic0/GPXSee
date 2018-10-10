#include <QLocale>
#include "tooltip.h"
#include "powergraphitem.h"


PowerGraphItem::PowerGraphItem(const Graph &graph, GraphType type,
  QGraphicsItem *parent) : GraphItem(graph, type, parent)
{
	qreal sum = 0;

	for (int j = 1; j < graph.size(); j++)
		sum += graph.at(j).y() * (graph.at(j).s() - graph.at(j-1).s());
	_avg = sum/graph.last().s();

	setToolTip(toolTip());
}

QString PowerGraphItem::toolTip() const
{
	ToolTip tt;
	QLocale l(QLocale::system());

	tt.insert(tr("Maximum"), l.toString(max(), 'f', 1)
	  + UNIT_SPACE + tr("W"));
	tt.insert(tr("Average"), l.toString(avg(), 'f', 1)
	  + UNIT_SPACE + tr("W"));

	return tt.toString();
}
