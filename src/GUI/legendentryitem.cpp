#include <QPainter>
#include <QCursor>
#include "font.h"
#include "legendentryitem.h"

#define PADDING 2

LegendEntryItem::LegendEntryItem(const QColor &color, const QString &text,
  QGraphicsItem *parent) : QGraphicsItem(parent), _color(color), _text(text)
{
	_textColor = Qt::black;
	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);

	QFontMetrics fm(_font);
	_boundingRect = QRectF(0, 0, FONT_SIZE * 2 + PADDING * 2
	  + fm.tightBoundingRect(text).width() + PADDING * 2,
	  FONT_SIZE + PADDING * 2);

	setCursor(Qt::ArrowCursor);
	setAcceptHoverEvents(true);
}

void LegendEntryItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFontMetrics fm(_font);

	painter->setRenderHint(QPainter::Antialiasing, false);

	painter->setFont(_font);
	painter->setBrush(_color);
	painter->setPen(QPen(Qt::black));
	painter->drawRect(PADDING, PADDING, FONT_SIZE * 2, FONT_SIZE);
	painter->setPen(QPen(_textColor));
	painter->drawText(FONT_SIZE * 2 + PADDING * 2 + PADDING,
	  PADDING + fm.ascent(), _text);

	//painter->setPen(Qt::red);
	//painter->setBrush(Qt::NoBrush);
	//painter->drawRect(boundingRect());
}

void LegendEntryItem::setTextColor(const QColor &color)
{
	_textColor = color;
	update();
}

void LegendEntryItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_font.setBold(true);
	setZValue(zValue() + 1.0);
	update();

	emit selected(true);
}

void LegendEntryItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);

	_font.setBold(false);
	setZValue(zValue() - 1.0);
	update();

	emit selected(false);
}
