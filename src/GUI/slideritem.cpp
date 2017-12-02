#include <QPainter>
#include "slideritem.h"


#define SIZE        10

SliderItem::SliderItem(QGraphicsItem *parent) : QGraphicsObject(parent)
{
	setFlag(ItemIsMovable);
	setFlag(ItemSendsGeometryChanges);

	_color = Qt::red;
}

QRectF SliderItem::boundingRect() const
{
	return QRectF(-SIZE/2, -_area.height(), SIZE, _area.height());
}

void SliderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(_color);
	painter->drawLine(0, 0, 0, -_area.height());

//	painter->drawRect(boundingRect());
}

QVariant SliderItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionChange && scene()) {
		QPointF pos = value.toPointF();

		if (!_area.contains(QRectF(pos, boundingRect().size()))) {
			pos.setX(qMin(_area.right(), qMax(pos.x(), _area.left())));
			pos.setY(qMin(_area.bottom(), qMax(pos.y(), _area.top()
			  + boundingRect().height())));

			return pos;
		}
	}

	if (change == ItemPositionHasChanged && scene())
		emit positionChanged(value.toPointF());

	return QGraphicsItem::itemChange(change, value);
}

void SliderItem::clear()
{
	_area = QRectF();
	setPos(QPointF());
}

void SliderItem::setArea(const QRectF &area)
{
	prepareGeometryChange();
	_area = area;
}

void SliderItem::setColor(const QColor &color)
{
	_color = color;
	update();
}
