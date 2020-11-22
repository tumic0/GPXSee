#include <QPainter>
#include <QFontMetrics>
#include "font.h"
#include "axislabelitem.h"


AxisLabelItem::AxisLabelItem(Type type, QGraphicsItem *parent)
  : QGraphicsItem(parent), _type(type)
{
	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);
}

void AxisLabelItem::setLabel(const QString& label, const QString &units)
{
	prepareGeometryChange();
	QFontMetrics fm(_font);
	_label = QString("%1 [%2]").arg(label, units.isEmpty() ? "-" : units);
	_labelBB = fm.tightBoundingRect(_label);
	updateBoundingRect();
	update();
}

void AxisLabelItem::updateBoundingRect()
{
	QFontMetrics fm(_font);

	if (_type == X)
		_boundingRect = QRectF(0, 0, _labelBB.width(), fm.height());
	else
		_boundingRect = QRectF(0, 0, fm.height(), _labelBB.width());
}

void AxisLabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFontMetrics fm(_font);

	painter->setFont(_font);

	if (_type == X) {
		painter->drawText(0, fm.height() - fm.descent(), _label);
	} else {
		painter->rotate(-90);
		painter->drawText(-_labelBB.width(), fm.height() - fm.descent(), _label);
		painter->rotate(90);
	}

	//painter->setRenderHint(QPainter::Antialiasing, false);
	//painter->setPen(Qt::red);
	//painter->drawRect(boundingRect());
}
