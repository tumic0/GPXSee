#include <QFontMetrics>
#include <QPainter>
#include "font.h"
#include "coordinatesitem.h"


CoordinatesItem::CoordinatesItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_format = DecimalDegrees;
	_units = Metric;
	_ele = NAN;
	_color = Qt::black;
	_bgColor = Qt::white;
	_drawBackground = false;
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

	if (_drawBackground) {
		painter->setPen(Qt::NoPen);
		QColor bc(_bgColor);
		bc.setAlpha(196);
		painter->setBrush(QBrush(bc));
		painter->drawRect(_boundingRect);
		painter->setBrush(Qt::NoBrush);
	}

	QFontMetrics fm(_font);
	painter->setFont(_font);
	painter->setPen(QPen(_color));
	painter->drawText(0, -fm.descent(), text());

	//painter->setPen(Qt::red);
	//painter->drawRect(boundingRect());
}

void CoordinatesItem::setCoordinates(const Coordinates &c, qreal elevation)
{
	prepareGeometryChange();

	_c = c;
	_ele = elevation;

	updateBoundingRect();
	update();
}

void CoordinatesItem::setFormat(CoordinatesFormat format)
{
	prepareGeometryChange();

	_format = format;
	updateBoundingRect();
}

void CoordinatesItem::setUnits(Units units)
{
	prepareGeometryChange();

	_units = units;
	updateBoundingRect();
}

void CoordinatesItem::setDigitalZoom(qreal zoom)
{
	_digitalZoom = zoom;
	setScale(pow(2, -_digitalZoom));
}

QString CoordinatesItem::text() const
{
	return (std::isnan(_ele))
	  ? Format::coordinates(_c, _format)
	  : Format::coordinates(_c, _format) + ", " + Format::elevation(_ele, _units);
}

void CoordinatesItem::updateBoundingRect()
{
	QFontMetrics fm(_font);

	QRectF br(fm.tightBoundingRect(text()));
	QRectF r1(br);
	QRectF r2(br);
	r1.moveTop(-fm.ascent());
	r2.moveBottom(-fm.descent());

	_boundingRect = r1 | r2;
}

void CoordinatesItem::setColor(const QColor &color)
{
	_color = color;
	update();
}

void CoordinatesItem::setBackgroundColor(const QColor &color)
{
	_bgColor = color;
	update();
}

void CoordinatesItem::drawBackground(bool draw)
{
	_drawBackground = draw;
	update();
}
