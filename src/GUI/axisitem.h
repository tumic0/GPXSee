#ifndef AXISITEM_H
#define AXISITEM_H

#include <QGraphicsItem>
#include <QVector>
#include <QLocale>
#include "common/range.h"

class AxisItem : public QGraphicsItem
{
public:
	enum Type {X, Y};

	AxisItem(Type type, QGraphicsItem *parent = 0);

	/* Note: The items position is at the 0 point of the axis line, not at the
	   top-left point of the bounding rect as usual */
	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setRange(const RangeF &range);
	void setSize(qreal size);
	void setZoom(qreal zoom) {_zoom = zoom;}

	QSizeF margin() const;
	QList<qreal> ticks() const;

private:
	struct Tick {
		double value;
		QRect boundingBox;
	};

	void updateBoundingRect();

	Type _type;
	RangeF _range;
	qreal _size;
	QVector<Tick> _ticks;
	QRectF _boundingRect;
	QFont _font;
	QLocale _locale;
	qreal _zoom;
};

#endif // AXISITEM_H
