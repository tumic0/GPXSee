#include <QFont>
#include <QPainter>
#include "font.h"
#include "infoitem.h"

#define PADDING     10

InfoItem::InfoItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	_font.setPixelSize(FONT_SIZE);
	_font.setFamily(FONT_FAMILY);
}

void InfoItem::updateBoundingRect()
{
	QFontMetrics fm(_font);
	qreal width = 0;

	for (QList<KV<QString, QString> >::const_iterator i = _list.constBegin();
	  i != _list.constEnd(); i++) {
		width += fm.horizontalAdvance(i->key() + ": " + i->value());
		if (i != _list.constEnd() - 1)
			width += PADDING;
	}

	_boundingRect = QRectF(0, 0, width, _list.isEmpty() ? 0 : fm.height());
}

void InfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QFontMetrics fm(_font);
	int width = 0;

	painter->setFont(_font);
	painter->setRenderHint(QPainter::Antialiasing, false);

	for (QList<KV<QString, QString> >::const_iterator i = _list.constBegin();
	  i != _list.constEnd(); i++) {
		QString text(i->key() + ": " + i->value());
		painter->drawText(width, fm.height() - fm.descent(), text);
		width += fm.horizontalAdvance(text);
		if (i != _list.constEnd() - 1) {
			width += PADDING;
			painter->save();
			painter->setPen(Qt::gray);
			painter->drawLine(width - PADDING/2, fm.descent(),
			  width - PADDING/2, fm.height() - fm.descent());
			painter->restore();
		}
	}

	//painter->setPen(Qt::red);
	//painter->drawRect(boundingRect());
}

int InfoItem::indexOf(const QString &key) const
{
	for (int i = 0; i < _list.size(); i++)
		if (_list.at(i).key() == key)
			return i;

	return -1;
}

void InfoItem::insert(const QString &key, const QString &value)
{
	int i;

	prepareGeometryChange();

	if ((i = indexOf(key)) < 0)
		_list.append(KV<QString, QString>(key, value));
	else
		_list[i] = KV<QString, QString>(key, value);

	updateBoundingRect();
	update();
}

void InfoItem::clear()
{
	prepareGeometryChange();
	_list.clear();
	updateBoundingRect();
}
