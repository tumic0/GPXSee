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

	qreal min() const {return -bounds().bottom();}
	qreal max() const {return -bounds().top();}
	qreal top() const {return _top;}

	const QMap<qreal, qreal> &map() const {return _map;}

private:
	QString toolTip() const;

	QMap<qreal, qreal> _map;
	qreal _top;
};

#endif // GEARRATIOGRAPHITEM_H
