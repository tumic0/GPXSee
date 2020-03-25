#include <QLocale>
#include "tooltip.h"
#include "cadencegraphitem.h"


CadenceGraphItem::CadenceGraphItem(const Graph &graph, GraphType type,
  int width, const QColor &color, QGraphicsItem *parent)
  : GraphItem(graph, type, width, color, Qt::SolidLine, parent)
{
}

QString CadenceGraphItem::info() const
{
	ToolTip tt;
	QLocale l(QLocale::system());

	tt.insert(tr("Maximum"), l.toString(max(), 'f', 1)
	  + UNIT_SPACE + tr("rpm"));
	tt.insert(tr("Average"), l.toString(avg(), 'f', 1)
	  + UNIT_SPACE + tr("rpm"));

	return tt.toString();
}
