#ifndef HEARTRATEGRAPHITEM_H
#define HEARTRATEGRAPHITEM_H

#include "graphitem.h"

class HeartRateGraphItem : public GraphItem
{
	Q_OBJECT

public:
	HeartRateGraphItem(const Graph &graph, GraphType type,
	  QGraphicsItem *parent = 0);

	qreal max() const {return -bounds().top();}
	qreal avg() const {return _avg;}

private:
	QString toolTip() const;

	qreal _avg;
};

#endif // HEARTRATEGRAPHITEM_H
