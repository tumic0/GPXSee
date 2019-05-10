#ifndef TEXTPATHITEM_H
#define TEXTPATHITEM_H

#include <QVector>
#include <QPainterPath>
#include "img.h"

class TextPathItem
{
public:
	TextPathItem() : _text(0), _font(0), _color(0) {}
	TextPathItem(const QPolygonF &line, const QString *label,
	  const QRect &tileRect, const QFont *font, const QColor *color);

	bool isValid() const {return !_path.isEmpty();}
	bool collides(const QVector<TextPathItem> &list) const;
	void paint(QPainter *painter) const;

private:
	const QString *_text;
	const QFont *_font;
	const QColor *_color;
	QPainterPath _path;
	QRectF _rect;
	QPainterPath _shape;
};

#endif // TEXTPATHITEM_H
