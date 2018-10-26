#include <QLocale>
#include "tooltip.h"
#include "cadencegraphitem.h"


CadenceGraphItem::CadenceGraphItem(const Graph &graph, GraphType type,
  QGraphicsItem *parent) : GraphItem(graph, type, parent)
{
	qreal sum = 0;
	_max = graph.first().y();

	for (int i = 1; i < graph.size(); i++) {
		qreal y = graph.at(i).y();
		sum += y * (graph.at(i).s() - graph.at(i-1).s());
		if (y > _max)
			_max = y;
	}
	_avg = sum/graph.last().s();

	setToolTip(toolTip());
}

QString CadenceGraphItem::toolTip() const
{
	ToolTip tt;
	QLocale l(QLocale::system());

	tt.insert(tr("Maximum"), l.toString(max(), 'f', 1)
	  + UNIT_SPACE + tr("rpm"));
	tt.insert(tr("Average"), l.toString(avg(), 'f', 1)
	  + UNIT_SPACE + tr("rpm"));

	return tt.toString();
}
