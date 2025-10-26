#ifndef TEXTPOINTITEM_H
#define TEXTPOINTITEM_H

#include <cmath>
#include "textitem.h"

class QFont;
class QImage;
class QColor;

class TextPointItem : public TextItem
{
public:
	enum Anchor {
		Center,
		Left,
		Right,
		Top,
		Bottom
	};

	TextPointItem(const QPoint &point, const QString *text, const QFont *font,
	  const QImage *img, const QColor *color, const QColor *haloColor,
	  const QColor *bgColor = 0, int padding = 0, double rotate = NAN,
	  Anchor textAnchor = Left, int maxWidth = 10);

	bool isValid() const {return !_rect.isEmpty();}

	QRectF boundingRect() const {return _rect;}
	QPainterPath shape() const {return _shape;}
	void paint(QPainter *painter) const;

	void setPos(const QPoint &point, int padding = 0);

protected:
	const QFont *_font;
	const QImage *_img;
	const QColor *_color, *_haloColor, *_bgColor;

private:
	void moveTextRect(const QPoint &pos, const QRectF &iconRect, int padding);

	double _rotate;
	Anchor _anchor;
	QRectF _rect, _textRect;
	QPainterPath _shape;
	QPointF _pos;
};

#endif // TEXTPOINTITEM_H
