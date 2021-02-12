#include <QPainter>
#include <QDateTime>
#include <QLocale>
#include "common/coordinates.h"
#include "font.h"
#include "markerinfoitem.h"


#define OFFSET 7

CoordinatesFormat MarkerInfoItem::_format = DecimalDegrees;

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
	_s1 = l.toString(date.date(), QLocale::ShortFormat);
	_s2 = l.toString(date.time(), QLocale::ShortFormat);

	updateBoundingRect();
}

void MarkerInfoItem::setCoordinates(const Coordinates &c)
{
	prepareGeometryChange();

	_s1 = Format::lat(c, _format);
	_s2 = Format::lon(c, _format);

	updateBoundingRect();
}

void MarkerInfoItem::updateBoundingRect()
{
	QFontMetrics fm(_font);

	qreal width = qMax(fm.boundingRect(_s1).width(),
	  fm.boundingRect(_s2).width());
	qreal height = 2 * fm.height() - 2*fm.descent();

	_boundingRect = QRectF(-OFFSET/2, -height/2, width + 1.5*OFFSET, height);
}

void MarkerInfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem
  *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QFontMetrics fm(_font);
	QRectF r1(OFFSET, -fm.height() + fm.descent(),
	  fm.boundingRect(_s1).width(), fm.height() - fm.descent());
	QRectF r2(OFFSET, 0, fm.boundingRect(_s2).width(), fm.height()
	  - fm.descent());

	painter->setPen(Qt::NoPen);
	QColor bc(painter->background().color());
	bc.setAlpha(196);
	painter->setBrush(QBrush(bc));
	painter->drawRect(r2);
	painter->drawRect(r1);
	painter->setBrush(Qt::NoBrush);

	painter->setFont(_font);
	painter->setPen(_color);

	painter->drawText(OFFSET, -fm.descent()/2, _s1);
	painter->drawText(OFFSET, fm.height() - fm.descent()*1.5, _s2);

	//painter->drawRect(boundingRect());
}

void MarkerInfoItem::setColor(const QColor &color)
{
	_color = color;
	update();
}
