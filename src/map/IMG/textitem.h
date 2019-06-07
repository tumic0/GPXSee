#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QList>
#include <QRectF>
#include <QPainterPath>

class QPainter;

class TextItem
{
public:
	virtual ~TextItem() {}

	virtual QPainterPath shape() const = 0;
	virtual QRectF boundingRect() const = 0;
	virtual void paint(QPainter *painter) const = 0;

	bool collides(const QList<TextItem*> &list) const;
};

#endif // TEXTITEM_H
