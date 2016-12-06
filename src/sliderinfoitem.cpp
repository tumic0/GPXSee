#include <QPainter>
#include "config.h"
#include "sliderinfoitem.h"


#define SIZE 5

SliderInfoItem::SliderInfoItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_side = Right;
}

void SliderInfoItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);

	_boundingRect = (_side == Right)
		? QRectF(-SIZE/2, 0, fm.width(_text) + SIZE, fm.height())
		: QRectF(-(fm.width(_text) + SIZE/2), 0, fm.width(_text) + SIZE,
		  fm.height());
}

void SliderInfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem
  *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);

	painter->setFont(font);
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(Qt::red);

	if (_side == Right)
		painter->drawText(SIZE, fm.height() - fm.descent(), _text);
	else
		painter->drawText(-(fm.width(_text) + SIZE/2),
		  fm.height() - fm.descent(), _text);
	painter->drawLine(QPointF(-SIZE/2, 0), QPointF(SIZE/2, 0));

	//painter->drawRect(boundingRect());
}

void SliderInfoItem::setText(const QString &text)
{
	prepareGeometryChange();
	_text = text;
	updateBoundingRect();
}

void SliderInfoItem::setSide(Side side)
{
	if (side == _side)
		return;

	prepareGeometryChange();
	_side = side;
	updateBoundingRect();
}
