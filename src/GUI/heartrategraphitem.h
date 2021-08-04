#ifndef HEARTRATEGRAPHITEM_H
#define HEARTRATEGRAPHITEM_H

#include "graphitem.h"

class HeartRateGraphItem : public GraphItem
{
	Q_OBJECT

public:
	HeartRateGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, QGraphicsItem *parent = 0);

	ToolTip info() const;
};

#endif // HEARTRATEGRAPHITEM_H
