#ifndef INFOITEM_H
#define INFOITEM_H

#include <QGraphicsItem>
#include <QList>
#include "kv.h"

class InfoItem : public QGraphicsItem
{
public:
	InfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QList<KV> &info() const {return _list;}

	void insert(const QString &key, const QString &value);
	void clear();
	bool isEmpty() {return _list.isEmpty();}

private:
	void updateBoundingRect();

	QList<KV> _list;
	QRectF _boundingRect;
	QFont _font;
};

#endif // INFOITEM_H
