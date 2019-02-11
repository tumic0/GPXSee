#ifndef CADENCEGRAPHITEM_H
#define CADENCEGRAPHITEM_H

#include "graphitem.h"

class CadenceGraphItem : public GraphItem
{
	Q_OBJECT

public:
	CadenceGraphItem(const Graph &graph, GraphType type,
	  QGraphicsItem *parent = 0);

private:
	QString toolTip() const;
};

#endif // CADENCEGRAPHITEM_H
