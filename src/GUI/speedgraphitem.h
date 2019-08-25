#ifndef SPEEDGRAPHITEM_H
#define SPEEDGRAPHITEM_H

#include "timetype.h"
#include "graphitem.h"

class SpeedGraphItem : public GraphItem
{
	Q_OBJECT

public:
	SpeedGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, qreal movingTime, QGraphicsItem *parent = 0);

	qreal avg() const {return _avg;}
	qreal mavg() const {return _mavg;}
	qreal max() const {return _max;}

	void setUnits(Units units);
	void setTimeType(TimeType type);

private:
	QString toolTip() const;

	qreal _avg, _mavg, _max;
	TimeType _timeType;
	Units _units;
};

#endif // SPEEDGRAPHITEM_H
