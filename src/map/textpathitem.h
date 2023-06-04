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
	  const QRect &tileRect, const QFont *font, const QColor *color,
	  const QColor *haloColor, const QImage *img = 0);
	TextPathItem(const QPainterPath &line, const QString *label,
	  const QRect &tileRect, const QFont *font, const QColor *color,
	  const QColor *haloColor, const QImage *img = 0);

	bool isValid() const {return !_path.isEmpty();}

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _rect;}
	void paint(QPainter *painter) const;

private:
	template<class T> void init(const T &line, const QRect &tileRect);

	const QFont *_font;
	const QColor *_color;
	const QColor *_haloColor;
	const QImage *_img;
	QPainterPath _path;
	QRectF _rect;
	QPainterPath _shape;
	bool _reverse;
};

#endif // TEXTPATHITEM_H
