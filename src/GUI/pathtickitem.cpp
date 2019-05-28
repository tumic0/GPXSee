#include <QPainter>
#include <QCursor>
#include "font.h"
#include "pathtickitem.h"


static QFont defaultFont()
{
	QFont font;
	font.setPixelSize(10);
	font.setFamily(FONT_FAMILY);
	font.setBold(true);

	return font;
}

QFont PathTickItem::_font = defaultFont();

PathTickItem::PathTickItem(const QRectF &tickRect, int value,
  QGraphicsItem *parent) : QGraphicsItem(parent), _tickRect(tickRect),
  _text(QString::number(value))
{
	_tickRect.moveCenter(QPointF(0, -_tickRect.height()/2.0 - 3));

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

QRectF PathTickItem::boundingRect() const
{
	return _tickRect.adjusted(0, 0, 0, 3);
}

void PathTickItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QPointF arrow[3] = {QPointF(0, 0), QPointF(3, -3), QPointF(-3, -3)};

	painter->setFont(_font);
	painter->setRenderHint(QPainter::Antialiasing, false);

	painter->setPen(Qt::white);
	painter->setBrush(_brush);
	painter->drawPolygon(arrow, 3);
	painter->drawRoundedRect(_tickRect, 1.5, 1.5);
	painter->drawText(_tickRect, Qt::AlignCenter, _text);

/*
	painter->setBrush(Qt::NoBrush);
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void PathTickItem::setPos(const QPointF &pos)
{
	/* For propper rounded rect rendering, the item must be positioned in the
	   middle of a pixel */
	QPoint p(pos.toPoint());
	QGraphicsItem::setPos(QPointF(p.x() - 0.5, p.y() - 0.5));
}

QRect PathTickItem::tickRect(int value)
{
	QFontMetrics fm(_font);
	return fm.boundingRect(QRect(), Qt::AlignCenter,
	  QString::number(qMax(value, 10))).adjusted(-2, 0, 2, 0);
}
