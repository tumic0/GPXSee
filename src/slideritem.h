#ifndef SLIDERITEM_H
#define SLIDERITEM_H

#include <QGraphicsItem>

class SliderItem : public QGraphicsObject
{
	Q_OBJECT

public:
	SliderItem(QGraphicsObject *parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	QRectF area() {return _area;}
	void setArea(const QRectF &area) {_area = area;}

	void clear();

signals:
	void positionChanged(const QPointF&);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QRectF _area;
};

#endif // SLIDERITEM_H
