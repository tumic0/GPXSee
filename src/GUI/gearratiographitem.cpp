#include <QMap>
#include <QLocale>
#include "tooltip.h"
#include "gearratiographitem.h"


GearRatioGraphItem::GearRatioGraphItem(const Graph &graph, GraphType type,
  int width, const QColor &color, QGraphicsItem *parent)
  : GraphItem(graph, type, width, color, Qt::SolidLine, parent)
{
	for (int i = 0; i < graph.size(); i++) {
		const GraphSegment &segment = graph.at(i);
		for (int j = 1; j < segment.size(); j++) {
			qreal dx = segment.at(j).s() - segment.at(j-1).s();
			_map.insert(segment.at(j).y(), _map.value(segment.at(j).y()) + dx);
		}
	}

	qreal key = NAN, val = NAN;
	for (QMap<qreal, qreal>::const_iterator it = _map.constBegin();
	  it != _map.constEnd(); ++it) {
		if (std::isnan(val) || it.value() > val) {
			val = it.value();
			key = it.key();
		}
	}
	_top = key;
}

QString GearRatioGraphItem::info() const
{
	ToolTip tt;
	QLocale l(QLocale::system());

	tt.insert(tr("Minimum"), l.toString(min(), 'f', 2));
	tt.insert(tr("Maximum"), l.toString(max(), 'f', 2));
	tt.insert(tr("Most used"), l.toString(top(), 'f', 2));

	return tt.toString();
}
