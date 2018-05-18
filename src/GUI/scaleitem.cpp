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

	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);

#ifndef Q_OS_MAC
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
#endif // Q_OS_MAC
}

void ScaleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFontMetrics fm(_font);
	QRect br;


	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setFont(_font);
	painter->setPen(QPen(Qt::black, BORDER_WIDTH));

	for (int i = 0; i < _ticks.size(); i++) {
		br = _ticks.at(i).boundingBox;
		painter->drawText(_width * i - br.width()/2, br.height() + 1,
		  QString::number(_ticks.at(i).value));
	}
	painter->drawText(_width * SEGMENTS + PADDING, SCALE_HEIGHT + PADDING
	  + br.height() + fm.descent(), _unitsStr);

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

void ScaleItem::updateCache()
{
	QFontMetrics fm(_font);

	_ticks = QVector<Tick>(SEGMENTS + 1);
	for (int i = 0; i < _ticks.size(); i++) {
		Tick &t = _ticks[i];
		t.value = _length * i;
		t.boundingBox = fm.tightBoundingRect(QString::number(t.value));
	}

	if (_units == Imperial)
		_unitsStr = _scale ? qApp->translate("ScaleItem", "mi")
		  : qApp->translate("ScaleItem", "ft");
	else if (_units == Nautical)
		_unitsStr = _scale ? qApp->translate("ScaleItem", "nmi")
		  : qApp->translate("ScaleItem", "ft");
	else
		_unitsStr = _scale ? qApp->translate("ScaleItem", "km")
		  : qApp->translate("ScaleItem", "m");
	_unitsBB = fm.tightBoundingRect(_unitsStr);

	QRect ss = _ticks.isEmpty() ? QRect() : _ticks.first().boundingBox;
	QRect es = _ticks.isEmpty() ? QRect() : _ticks.last().boundingBox;
	_boundingRect = QRectF(-ss.width()/2, 0, _width * SEGMENTS + ss.width()/2
	  + qMax(_unitsBB.width() + PADDING, es.width()/2) + 1, SCALE_HEIGHT
	  + PADDING + ss.height() + 2*fm.descent());
}

void ScaleItem::setResolution(qreal res)
{
	prepareGeometryChange();
	_res = res;
	computeScale();
	updateCache();
	update();
}

void ScaleItem::setUnits(Units units)
{
	prepareGeometryChange();
	_units = units;
	computeScale();
	updateCache();
	update();
}

void ScaleItem::setDigitalZoom(qreal zoom)
{
	prepareGeometryChange();
	_digitalZoom = zoom;
	computeScale();
	updateCache();
	update();

	setScale(pow(2, -_digitalZoom));
}
