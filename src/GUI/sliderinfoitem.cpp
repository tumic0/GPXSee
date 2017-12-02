#include <QPainter>
#include "config.h"
#include "sliderinfoitem.h"


#define SIZE 5

SliderInfoItem::SliderInfoItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_side = Right;
	_color = Qt::red;
}

void SliderInfoItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);

	qreal width = qMax(fm.width(_x), fm.width(_y));
	qreal height = 2 * fm.height() - 2*fm.descent();

	_boundingRect = (_side == Right)
	  ? QRectF(-SIZE/2, -height/2, width + 1.5*SIZE, height)
	  : QRectF(-(width + SIZE), -height/2, width + 1.5*SIZE, height);
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
	QRectF rx, ry;


	qreal width = qMax(fm.width(_x), fm.width(_y));
	if (_side == Right) {
		ry = QRectF(SIZE, -fm.height() + fm.descent(), fm.width(_y),
		  fm.height() - fm.descent());
		rx = QRectF(SIZE, 0, fm.width(_x), fm.height()
		  - fm.descent());
	} else {
		ry = QRectF(-(width + SIZE), -fm.height() + fm.descent(), fm.width(_y),
		  fm.height() - fm.descent());
		rx = QRectF(-(width + SIZE), 0, fm.width(_x), fm.height()
		  - fm.descent());
	}

	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(QColor(255, 255, 255, 196)));
	painter->drawRect(ry);
	painter->drawRect(rx);
	painter->setBrush(Qt::NoBrush);

	painter->setFont(font);
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(_color);

	if (_side == Right) {
		painter->drawText(SIZE, -fm.descent()/2, _y);
		painter->drawText(SIZE, fm.height() - fm.descent()*1.5, _x);
	} else {
		painter->drawText(-(width + SIZE), -fm.descent()/2, _y);
		painter->drawText(-(width + SIZE), fm.height() - fm.descent()*1.5, _x);
	}
	painter->drawLine(QPointF(-SIZE/2, 0), QPointF(SIZE/2, 0));

	//painter->drawRect(boundingRect());
}

void SliderInfoItem::setText(const QString &x, const QString &y)
{
	prepareGeometryChange();
	_x = x; _y = y;
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

void SliderInfoItem::setColor(const QColor &color)
{
	_color = color;
	update();
}
