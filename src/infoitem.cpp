#include <QFont>
#include <QPainter>
#include "config.h"
#include "infoitem.h"


#define PADDING     10

InfoItem::InfoItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{

}

QRectF InfoItem::boundingRect() const
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QList<KV>::const_iterator i;
	int width = 0;

	if (_list.isEmpty())
		return QRectF();

	for (i = _list.constBegin(); i != _list.constEnd(); i++) {
		width += fm.width(i->key + ": ");
		width += fm.width(i->value) + ((i == _list.end() - 1) ? 0 : PADDING);
	}

	return QRectF(0, 0, width, fm.height());
}

void InfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	painter->setFont(font);
	QFontMetrics fm(font);
	QList<KV>::const_iterator i;
	int width = 0;

	for (i = _list.constBegin(); i != _list.constEnd(); i++) {
		painter->drawText(width, fm.height() - fm.descent(), i->key + ": ");
		width += fm.width(i->key + ": ");
		painter->drawText(width, fm.height() - fm.descent(), i->value);
		width += fm.width(i->value) + ((i == _list.end() - 1) ? 0 : PADDING);
		if (i != _list.end() - 1) {
			painter->save();
			painter->setPen(Qt::gray);
			painter->drawLine(width - PADDING/2, fm.descent(),
			  width - PADDING/2, fm.height() - fm.descent());
			painter->restore();
		}
	}

/*
	painter->setPen(Qt::red);
	painter->drawRect(boundingRect());
*/
}

void InfoItem::insert(const QString &key, const QString &value)
{
	KV kv(key, value);
	int i;

	if ((i = _list.indexOf(kv)) < 0)
		_list.append(kv);
	else
		_list[i] = kv;

	prepareGeometryChange();
}
