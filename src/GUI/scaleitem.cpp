#include <cmath>
#include <QPainter>
#include "config.h"
#include "nicenum.h"
#include "scaleitem.h"


#define BORDER_WIDTH   1
#define SCALE_WIDTH    132
#define SCALE_HEIGHT   5
#define SEGMENTS       3
#define PADDING        4


ScaleItem::ScaleItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_units = Metric;
	_res = 1.0;
	_digitalZoom = 0;

#ifndef Q_OS_MAC
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif // Q_OS_MAC
}

void ScaleItem::updateBoundingRect()
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect ss, es, us;

	ss = fm.tightBoundingRect(QString::number(0));
	es = fm.tightBoundingRect(QString::number(_length * SEGMENTS));
	us = fm.tightBoundingRect(units());

	_boundingRect = QRectF(-ss.width()/2, 0, _width * SEGMENTS + ss.width()/2
	  + qMax(us.width() + PADDING, es.width()/2) + 1, SCALE_HEIGHT + PADDING
	  + ss.height() + 2*fm.descent());
}

void ScaleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QRect br;
	QPen pen = QPen(Qt::black, BORDER_WIDTH);


	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setFont(font);
	painter->setPen(pen);

	for (int i = 0; i <= SEGMENTS; i++) {
		QString label = QString::number(_length * i);
		br = fm.tightBoundingRect(label);
		painter->drawText(_width * i - br.width()/2, br.height() + 1, label);
	}
	painter->drawText(_width * SEGMENTS + PADDING, SCALE_HEIGHT + PADDING
	  + br.height() + fm.descent(), units());

	painter->drawRect(QRectF(0, br.height() + PADDING, SEGMENTS * _width,
	  SCALE_HEIGHT));
	for (int i = 0; i < SEGMENTS; i += 2)
		painter->fillRect(QRectF(i * _width, br.height() + PADDING, _width,
		  SCALE_HEIGHT), Qt::black);

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

QString ScaleItem::units() const
{
	if (_units == Imperial)
		return _scale ? qApp->translate("ScaleItem", "mi")
		  : qApp->translate("ScaleItem", "ft");
	else if (_units == Nautical)
		return _scale ? qApp->translate("ScaleItem", "nmi")
		  : qApp->translate("ScaleItem", "ft");
	else
		return _scale ? qApp->translate("ScaleItem", "km")
		  : qApp->translate("ScaleItem", "m");
}

void ScaleItem::computeScale()
{
	qreal res = _res * pow(2, -_digitalZoom);

	if (_units == Imperial) {
		_length = niceNum((res * M2FT * SCALE_WIDTH) / SEGMENTS, 1);
		if (_length >= MIINFT) {
			_length = niceNum((res * M2MI * SCALE_WIDTH) / SEGMENTS, 1);
			_width = (_length / (res * M2MI));
			_scale = true;
		} else {
			_width = (_length / (res * M2FT));
			_scale = false;
		}
	} else if (_units == Nautical) {
		_length = niceNum((res * M2FT * SCALE_WIDTH) / SEGMENTS, 1);
		if (_length >= NMIINFT) {
			_length = niceNum((res * M2NMI * SCALE_WIDTH) / SEGMENTS, 1);
			_width = (_length / (res * M2NMI));
			_scale = true;
		} else {
			_width = (_length / (res * M2FT));
			_scale = false;
		}
	} else {
		_length = niceNum((res * SCALE_WIDTH) / SEGMENTS, 1);
		if (_length >= KMINM) {
			_length *= M2KM;
			_width = (_length / (res * M2KM));
			_scale = true;
		} else {
			_width = (_length / res);
			_scale = false;
		}
	}
}

void ScaleItem::setResolution(qreal res)
{
	prepareGeometryChange();
	_res = res;
	computeScale();
	updateBoundingRect();
	update();
}

void ScaleItem::setUnits(enum Units units)
{
	prepareGeometryChange();
	_units = units;
	computeScale();
	updateBoundingRect();
	update();
}

void ScaleItem::setDigitalZoom(qreal zoom)
{
	prepareGeometryChange();
	_digitalZoom = zoom;
	computeScale();
	updateBoundingRect();
	update();

	setScale(pow(2, -_digitalZoom));
}
