#include <cmath>
#include <QPainter>
#include "font.h"
#include "legenditem.h"

#define PADDING 4

LegendItem::LegendItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_color = Qt::black;
	_bgColor = Qt::white;
	_drawBackground = false;
	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);
}

void LegendItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setRenderHint(QPainter::Antialiasing, false);

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

	for (int i = 0; i < _items.size(); i++) {
		const Item &itm = _items.at(i);

		painter->setBrush(itm.color);
		painter->setPen(QPen(Qt::black));
		painter->drawRect(0, i * (FONT_SIZE + PADDING), FONT_SIZE * 2, FONT_SIZE);
		painter->setPen(QPen(_color));
		painter->drawText(FONT_SIZE * 2 + PADDING, i * (FONT_SIZE + PADDING)
		  + fm.ascent(), itm.text);
	}

	//painter->setPen(Qt::red);
	//painter->setBrush(Qt::NoBrush);
	//painter->drawRect(boundingRect());
}

void LegendItem::addItem(const QColor &color, const QString &text)
{
	prepareGeometryChange();

	_items.append(Item(color, text));

	QFontMetrics fm(_font);
	qreal maxWidth = qMax((int)_boundingRect.width() - (FONT_SIZE * 2 + PADDING),
	  fm.tightBoundingRect(text).width());
	_boundingRect = QRectF(0, 0, FONT_SIZE * 2 + PADDING + maxWidth,
	  _items.size() * (FONT_SIZE + PADDING) - (PADDING - 1));
}

void LegendItem::setColor(const QColor &color)
{
	_color = color;
	update();
}

void LegendItem::setBackgroundColor(const QColor &color)
{
	_bgColor = color;
	update();
}

void LegendItem::drawBackground(bool draw)
{
	_drawBackground = draw;
	update();
}

void LegendItem::clear()
{
	prepareGeometryChange();
	_items.clear();
	_boundingRect = QRectF();
}

void LegendItem::setDigitalZoom(qreal zoom)
{
	setScale(pow(2, -zoom));
}
