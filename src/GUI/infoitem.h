#ifndef INFOITEM_H
#define INFOITEM_H

#include <QGraphicsItem>
#include <QList>
#include "common/kv.h"

class InfoItem : public QGraphicsItem
{
public:
	InfoItem(QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	const QList<KV<QString, QString> > &info() const {return _list;}

	void insert(const QString &key, const QString &value);
	void clear();
	bool isEmpty() {return _list.isEmpty();}

private:
	void updateBoundingRect();
	int indexOf(const QString &key) const;

	QList<KV<QString, QString> > _list;
	QRectF _boundingRect;
	QFont _font;
};

#endif // INFOITEM_H
