#include <QFontMetrics>
#include <QPainter>
#include "font.h"
#include "coordinatesitem.h"


CoordinatesItem::CoordinatesItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_format = DecimalDegrees;

	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);

	_digitalZoom = 0;

	setAcceptHoverEvents(true);

	updateBoundingRect();
}

void CoordinatesItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (!_c.isValid())
		return;

	QFontMetrics fm(_font);
	painter->setFont(_font);
	painter->setPen(QPen(Qt::black));
	painter->drawText(0, -fm.descent(), Format::coordinates(_c, _format));

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void CoordinatesItem::setCoordinates(const Coordinates &c)
{
	_c = c;
	update();
}

void CoordinatesItem::setFormat(const CoordinatesFormat &format)
{
	prepareGeometryChange();

	_format = format;
	updateBoundingRect();
}

void CoordinatesItem::setDigitalZoom(qreal zoom)
{
	_digitalZoom = zoom;
	setScale(pow(2, -_digitalZoom));
}

void CoordinatesItem::updateBoundingRect()
{
	QFontMetrics fm(_font);
	_boundingRect = fm.tightBoundingRect(Format::coordinates(
	  Coordinates(-180, -90), _format));
	_boundingRect.moveBottom(-fm.descent());
}
