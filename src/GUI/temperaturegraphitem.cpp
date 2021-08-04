#include <QLocale>
#include "tooltip.h"
#include "temperaturegraphitem.h"


TemperatureGraphItem::TemperatureGraphItem(const Graph &graph, GraphType type,
  int width, const QColor &color, QGraphicsItem *parent)
  : GraphItem(graph, type, width, color, Qt::SolidLine, parent)
{
	_min = GraphItem::min();
	_max = GraphItem::max();
	_avg = GraphItem::avg();
}

ToolTip TemperatureGraphItem::info() const
{
	ToolTip tt;
	qreal scale = (_units == Metric) ? 1.0 : C2FS;
	qreal offset = (_units == Metric) ? 0 : C2FO;
	QString su = (_units == Metric) ?
	  QChar(0x00B0) + tr("C") : QChar(0x00B0) + tr("F");
	QLocale l(QLocale::system());

	tt.insert(tr("Average"), l.toString(avg() * scale + offset, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Maximum"), l.toString(max() * scale + offset, 'f', 1)
	  + UNIT_SPACE + su);
	tt.insert(tr("Minimum"), l.toString(min() * scale + offset, 'f', 1)
	  + UNIT_SPACE + su);

	return tt;
}
