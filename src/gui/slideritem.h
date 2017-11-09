#ifndef SLIDERITEM_H
#define SLIDERITEM_H

#include <QGraphicsObject>

class SliderItem : public QGraphicsObject
{
	Q_OBJECT

public:
	SliderItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QRectF &area() const {return _area;}
	void setArea(const QRectF &area);

	void clear();

signals:
	void positionChanged(const QPointF&);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QRectF _area;
};

#endif // SLIDERITEM_H
