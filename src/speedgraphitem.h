#ifndef SPEEDGRAPHITEM_H
#define SPEEDGRAPHITEM_H

#include "timetype.h"
#include "graphitem.h"

class SpeedGraphItem : public GraphItem
{
	Q_OBJECT

public:
	SpeedGraphItem(const Graph &graph, GraphType type, qreal movingTime,
	  QGraphicsItem *parent = 0);

	qreal max() const {return -bounds().top();}
	qreal avg() const {return _avg;}
	qreal mavg() const {return _mavg;}

	void setUnits(Units units);
	void setTimeType(TimeType type);

private:
	QString toolTip() const;

	qreal _avg, _mavg;

	Units _units;
	TimeType _timeType;
};

#endif // SPEEDGRAPHITEM_H
