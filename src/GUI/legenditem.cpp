#include <cmath>
#include <QPainter>
#include <QFileInfo>
#include "font.h"
#include "pathitem.h"
#include "graphitem.h"
#include "planeitem.h"
#include "legendentryitem.h"
#include "legenditem.h"

LegendItem::LegendItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_bgColor = Qt::white;
	_drawBackground = false;
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
	}

	//painter->setPen(Qt::red);
	//painter->setBrush(Qt::NoBrush);
	//painter->drawRect(boundingRect());
}

void LegendItem::addItem(PathItem *item)
{
	prepareGeometryChange();

	LegendEntryItem *li = new LegendEntryItem(item->color(),
	  item->name().isEmpty() ? QFileInfo(item->file()).fileName() : item->name(),
	  this);
	li->setPos(0, _items.size() * li->boundingRect().height());

	_items.append(li);

	_boundingRect = QRectF(0, 0, qMax(_boundingRect.width(),
	  li->boundingRect().width()), _items.size() * li->boundingRect().height());

	QObject::connect(li, &LegendEntryItem::selected, item, &PathItem::hoverAll);
}

void LegendItem::addItem(PlaneItem *item)
{
	if (item->name().isEmpty())
		return;

	prepareGeometryChange();

	LegendEntryItem *li = new LegendEntryItem(item->color(), item->name(), this);
	li->setPos(0, _items.size() * li->boundingRect().height());

	_items.append(li);

	_boundingRect = QRectF(0, 0, qMax(_boundingRect.width(),
	  li->boundingRect().width()), _items.size() * li->boundingRect().height());

	QObject::connect(li, &LegendEntryItem::selected, item, &PlaneItem::hover);
}

void LegendItem::setColor(const QColor &color)
{
	for (int i = 0; i < _items.size(); i++)
		_items.at(i)->setTextColor(color);
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

	qDeleteAll(_items);
	_items.clear();
	_boundingRect = QRectF();
}

void LegendItem::setDigitalZoom(qreal zoom)
{
	setScale(pow(2, -zoom));
}
