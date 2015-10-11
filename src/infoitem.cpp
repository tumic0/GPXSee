#include <QFont>
#include <QPainter>
#include "infoitem.h"

#define FONT_FAMILY "Arial"
#define FONT_SIZE   12
#define PADDING     10

InfoItem::InfoItem()
{

}

QRectF InfoItem::boundingRect() const
{
	QFont font;
	font.setPixelSize(FONT_SIZE);
	font.setFamily(FONT_FAMILY);
	QFontMetrics fm(font);
	QMap<QString, QString>::const_iterator i;
	int width = 0;

	if (_map.isEmpty())
		return QRectF();

	for (i = _map.constBegin(); i != _map.constEnd(); i++) {
		width += fm.width(i.key() + ": ");
		width += fm.width(i.value()) + ((i == _map.end() - 1) ? 0 : PADDING);
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
	QMap<QString, QString>::const_iterator i;
	int width = 0;

	for (i = _map.constBegin(); i != _map.constEnd(); i++) {
		painter->drawText(width, fm.height() - fm.descent(), i.key() + ": ");
		width += fm.width(i.key() + ": ");
		painter->drawText(width, fm.height() - fm.descent(), i.value());
		width += fm.width(i.value()) + ((i == _map.end() - 1) ? 0 : PADDING);
		if (i != _map.end() - 1) {
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
	_map.insert(key, value);
}
