#ifndef TEXTPATHITEM_H
#define TEXTPATHITEM_H

#include <QVector>
#include <QPainterPath>
#include "textitem.h"

class TextPathItem : public TextItem
{
public:
	TextPathItem() : TextItem(0), _font(0), _color(0) {}
	TextPathItem(const QPolygonF &line, const QString *label,
	  const QRect &tileRect, const QFont *font, const QColor *color);

	bool isValid() const {return !_path.isEmpty();}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _rect;}
	void paint(QPainter *painter) const;

private:
	const QFont *_font;
	const QColor *_color;
	QPainterPath _path;
	QRectF _rect;
	QPainterPath _shape;
};

#endif // TEXTPATHITEM_H
