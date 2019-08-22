#include <QMap>
#include <QLocale>
#include "tooltip.h"
#include "gearratiographitem.h"


GearRatioGraphItem::GearRatioGraphItem(const Graph &graph, GraphType type,
  QGraphicsItem *parent) : GraphItem(graph, type, parent), _top(NAN)
{
	for (int i = 0; i < graph.size(); i++) {
		const GraphSegment &segment = graph.at(i);
		for (int j = 1; j < segment.size(); j++) {
			qreal dx = segment.at(j).s() - segment.at(j-1).s();
			_map.insert(segment.at(j).y(), _map.value(segment.at(j).y()) + dx);
		}
	}

	setToolTip(toolTip());
}

QString GearRatioGraphItem::toolTip() const
{
	ToolTip tt;
	QLocale l(QLocale::system());

	tt.insert(tr("Minimum"), l.toString(min(), 'f', 2));
	tt.insert(tr("Maximum"), l.toString(max(), 'f', 2));
	tt.insert(tr("Most used"), l.toString(top(), 'f', 2));

	return tt.toString();
}
