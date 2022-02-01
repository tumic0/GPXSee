#include <cmath>
#include <QFont>
#include <QPainter>
#include <QLocale>
#include <QApplication>
#include "font.h"
#include "motioninfoitem.h"


#define DEGREE_UNIT QString::fromUtf8("\xC2\xB0")

MotionInfoItem::MotionInfoItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_units = Metric;
	_bearing = NAN;
	_speed = NAN;
	_verticalSpeed = NAN;
	_color = Qt::black;
	_bgColor = Qt::white;
	_drawBackground = false;
	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);
	_digitalZoom = 0;

	setAcceptHoverEvents(true);

	updateBoundingRect();
}

void MotionInfoItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (std::isnan(_bearing) && std::isnan(_speed) && std::isnan(_verticalSpeed))
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

void MotionInfoItem::setInfo(qreal bearing, qreal speed, qreal verticalSpeed)
{
	prepareGeometryChange();

	_bearing = bearing;
	_speed = speed;
	_verticalSpeed = verticalSpeed;

	updateBoundingRect();
	update();
}

void MotionInfoItem::setUnits(Units units)
{
	prepareGeometryChange();

	_units = units;
	updateBoundingRect();
}

void MotionInfoItem::setDigitalZoom(qreal zoom)
{
	_digitalZoom = zoom;
	setScale(pow(2, -_digitalZoom));
}

QString MotionInfoItem::speed(const QLocale &l) const
{
	if (_units == Nautical)
		return l.toString(MS2KN * _speed, 'f', 1) + UNIT_SPACE
		  + qApp->translate("MotionInfoItem", "kn");
	else if (_units == Imperial)
		return l.toString(MS2MIH * _speed, 'f', 1) + UNIT_SPACE
		  + qApp->translate("MotionInfoItem", "mi/h");
	else
		return l.toString(MS2KMH * _speed, 'f', 1) + UNIT_SPACE
		  + qApp->translate("MotionInfoItem", "km/h");
}

QString MotionInfoItem::verticalSpeed(const QLocale &l) const
{
	if (_units == Nautical || _units == Imperial)
		return l.toString(MS2FTMIN * _verticalSpeed, 'f', 1) + UNIT_SPACE
		  + qApp->translate("MotionInfoItem", "ft/min");
	else
		return l.toString(MS2MMIN * _verticalSpeed, 'f', 1) + UNIT_SPACE
		  + qApp->translate("MotionInfoItem", "m/min");
}

static QString bearing(qreal val, const QLocale &l)
{
	return l.toString(val, 'f', 0) + DEGREE_UNIT;
}

QString MotionInfoItem::text() const
{
	QLocale l(QLocale::system());
	QString str;

	if (!std::isnan(_bearing))
		str += bearing(_bearing, l);
	if (!std::isnan(_speed)) {
		if (!str.isEmpty())
			str += ", ";
		str += speed(l);
	} if (!std::isnan(_verticalSpeed)) {
		if (!str.isEmpty())
			str += ", ";
		str += verticalSpeed(l);
	}

	return str;
}

void MotionInfoItem::updateBoundingRect()
{
	QFontMetrics fm(_font);

	QRectF br(fm.tightBoundingRect(text()));
	QRectF r1(br);
	QRectF r2(br);
	r1.moveTop(-fm.ascent());
	r2.moveBottom(-fm.descent());

	_boundingRect = r1 | r2;
}

void MotionInfoItem::setColor(const QColor &color)
{
	_color = color;
	update();
}

void MotionInfoItem::setBackgroundColor(const QColor &color)
{
	_bgColor = color;
	update();
}

void MotionInfoItem::drawBackground(bool draw)
{
	_drawBackground = draw;
	update();
}
