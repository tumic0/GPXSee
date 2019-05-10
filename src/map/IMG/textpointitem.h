#ifndef TEXTPOINTITEM_H
#define TEXTPOINTITEM_H

#include <QRect>
#include <QString>
#include <QVector>

class QPainter;
class QFont;
class QImage;
class QColor;

class TextPointItem
{
public:
	TextPointItem() : _text(0), _font(0), _img(0) {}
	TextPointItem(const QPoint &point, const QString *text, const QFont *font,
	  const QImage *img, const QColor *color);

	bool collides(const QVector<TextPointItem> &list) const;
	void paint(QPainter *painter) const;

private:
	const QString *_text;
	const QFont *_font;
	const QImage *_img;
	const QColor *_color;
	QRect _rect, _textRect;
};

#endif // TEXTPOINTITEM_H
