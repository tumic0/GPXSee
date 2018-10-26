#ifndef GEARRATIOGRAPHITEM_H
#define GEARRATIOGRAPHITEM_H

#include <QMap>
#include "graphitem.h"

class GearRatioGraphItem : public GraphItem
{
	Q_OBJECT

public:
	GearRatioGraphItem(const Graph &graph, GraphType type,
	  QGraphicsItem *parent = 0);

	qreal min() const {return _min;}
	qreal max() const {return _max;}
	qreal top() const {return _top;}

	const QMap<qreal, qreal> &map() const {return _map;}

private:
	QString toolTip() const;

	QMap<qreal, qreal> _map;
	qreal _top, _min, _max;
};

#endif // GEARRATIOGRAPHITEM_H
