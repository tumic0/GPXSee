#ifndef HEARTRATEGRAPHITEM_H
#define HEARTRATEGRAPHITEM_H

#include "graphitem.h"

class HeartRateGraphItem : public GraphItem
{
	Q_OBJECT

public:
	HeartRateGraphItem(const Graph &graph, GraphType type,
	  QGraphicsItem *parent = 0);

private:
	QString toolTip() const;
};

#endif // HEARTRATEGRAPHITEM_H
