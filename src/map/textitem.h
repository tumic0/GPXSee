#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QList>
#include <QRectF>
#include <QPainterPath>

class QPainter;

class TextItem
{
public:
	TextItem(const QString *text) : _text(text) {}
	virtual ~TextItem() {}

	virtual QPainterPath shape() const = 0;
	virtual QRectF boundingRect() const = 0;
	virtual void paint(QPainter *painter) const = 0;

	const QString *text() const {return _text;}
	bool collides(const QList<TextItem*> &list) const;

protected:
	const QString *_text;
};

#endif // TEXTITEM_H
