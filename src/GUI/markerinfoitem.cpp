#include <QPainter>
#include <QDateTime>
#include <QLocale>
#include "font.h"
#include "markerinfoitem.h"


#define OFFSET 7

MarkerInfoItem::MarkerInfoItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_color = Qt::red;

	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);
}

void MarkerInfoItem::setDate(const QDateTime &date)
{
	prepareGeometryChange();

	QLocale l;
	_date = l.toString(date.date(), QLocale::ShortFormat);
	_time = l.toString(date.time(), QLocale::ShortFormat);

	updateBoundingRect();
}

void MarkerInfoItem::updateBoundingRect()
{
	QFontMetrics fm(_font);

	qreal width = qMax(fm.boundingRect(_date).width(),
	  fm.boundingRect(_time).width());
	qreal height = 2 * fm.height() - 2*fm.descent();

	_boundingRect = QRectF(-OFFSET/2, -height/2, width + 1.5*OFFSET, height);
}

void MarkerInfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem
  *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QFontMetrics fm(_font);
	QRectF rd(OFFSET, -fm.height() + fm.descent(),
	  fm.boundingRect(_date).width(), fm.height() - fm.descent());
	QRectF rt(OFFSET, 0, fm.boundingRect(_time).width(), fm.height()
	  - fm.descent());

	painter->setPen(Qt::NoPen);
	QColor bc(painter->background().color());
	bc.setAlpha(196);
	painter->setBrush(QBrush(bc));
	painter->drawRect(rt);
	painter->drawRect(rd);
	painter->setBrush(Qt::NoBrush);

	painter->setFont(_font);
	painter->setPen(_color);

	painter->drawText(OFFSET, -fm.descent()/2, _date);
	painter->drawText(OFFSET, fm.height() - fm.descent()*1.5, _time);

	//painter->drawRect(boundingRect());
}

void MarkerInfoItem::setColor(const QColor &color)
{
	_color = color;
	update();
}
