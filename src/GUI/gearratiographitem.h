#ifndef GEARRATIOGRAPHITEM_H
#define GEARRATIOGRAPHITEM_H

#include <QMap>
#include "graphitem.h"

class GearRatioGraphItem : public GraphItem
{
	Q_OBJECT

public:
	GearRatioGraphItem(const Graph &graph, GraphType type, int width,
	  const QColor &color, QGraphicsItem *parent = 0);

	qreal top() const {return _top;}
	const QMap<qreal, qreal> &map() const {return _map;}

	QString info() const;

private:
	QMap<qreal, qreal> _map;
	qreal _top;
};

#endif // GEARRATIOGRAPHITEM_H
