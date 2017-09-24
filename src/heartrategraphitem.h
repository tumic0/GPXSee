#ifndef HEARTRATEGRAPHITEM_H
#define HEARTRATEGRAPHITEM_H

#include "graphitem.h"

class HeartRateGraphItem : public GraphItem
{
public:
	HeartRateGraphItem(const Graph &graph, QGraphicsItem *parent = 0);

	qreal max() const {return -bounds().top();}
	qreal avg() const {return _avg;}

private:
	QString toolTip() const;

	qreal _avg;
};

#endif // HEARTRATEGRAPHITEM_H
