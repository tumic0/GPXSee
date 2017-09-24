#ifndef SPEEDGRAPHITEM_H
#define SPEEDGRAPHITEM_H

#include "graphitem.h"

class SpeedGraphItem : public GraphItem
{
public:
	SpeedGraphItem(const Graph &graph, qreal movingTime,
	  QGraphicsItem *parent = 0);

	qreal max() const {return -bounds().top();}
	qreal avg() const {return _avg;}
	qreal mavg() const {return _mavg;}

	void setUnits(Units units);

private:
	QString toolTip(Units units) const;

	qreal _avg, _mavg;
};

#endif // SPEEDGRAPHITEM_H
