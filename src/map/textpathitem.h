#ifndef TEXTPATHITEM_H
#define TEXTPATHITEM_H

#include "textitem.h"

class QFont;
class QImage;
class QColor;

class TextPathItem : public TextItem
{
public:
	TextPathItem(const QPolygonF &line, const QString *label,
	  const QRect &tileRect, const QFont *font, const QColor *color,
	  const QColor *haloColor, const QImage *img = 0, bool rotate = true);
	TextPathItem(const QPainterPath &line, const QString *label,
	  const QRect &tileRect, const QFont *font, const QColor *color,
	  const QColor *haloColor, const QImage *img = 0, bool rotate = true);

	bool isValid() const {return !_path.isEmpty();}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _rect;}
	void paint(QPainter *painter) const;

private:
	template<class T> void init(const T &line, const QRect &tileRect);

	const QFont *_font;
	const QColor *_color, *_haloColor;
	const QImage *_img;
	QRectF _rect;
	QPainterPath _path, _shape;
	bool _rotate, _reverse;
};

#endif // TEXTPATHITEM_H
