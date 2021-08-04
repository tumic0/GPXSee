#ifndef POWERGRAPHITEM_H
#define POWERGRAPHITEM_H

#include "graphitem.h"

class PowerGraphItem : public GraphItem
{
	Q_OBJECT

public:
	PowerGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, QGraphicsItem *parent = 0);

	ToolTip info() const;
};

#endif // POWERGRAPHITEM_H
