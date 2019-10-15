#ifndef ELEVATIONGRAPHITEM_H
#define ELEVATIONGRAPHITEM_H

#include "graphitem.h"

class ElevationGraphItem : public GraphItem
{
	Q_OBJECT

public:
	ElevationGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, QGraphicsItem *parent = 0);

	qreal ascent() const {return _ascent;}
	qreal descent() const {return _descent;}
	qreal max() const {return _max;}
	qreal min() const {return _min;}

	QString info() const;

private:
	qreal _ascent, _descent, _min, _max;
};

#endif // ELEVATIONGRAPHITEM_H
