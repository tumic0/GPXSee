#ifndef POIITEM_H
#define POIITEM_H

#include <QGraphicsItem>

class POIItem : public QGraphicsItem
{
public:
	POIItem(const QString &text);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

private:
	void updateBoundingRect();

	QString _text;
	QRectF _boundingRect;
};

#endif // POIITEM_H
