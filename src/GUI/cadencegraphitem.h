#ifndef CADENCEGRAPHITEM_H
#define CADENCEGRAPHITEM_H

#include "graphitem.h"

class CadenceGraphItem : public GraphItem
{
	Q_OBJECT

public:
	CadenceGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, QGraphicsItem *parent = 0);

	ToolTip info(bool extended) const;
};

#endif // CADENCEGRAPHITEM_H
