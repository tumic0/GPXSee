#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)
#define MAX_TEXT_WIDTH 8
#define MIN_BOX_WIDTH 2


static void expand(QRect &rect, int width)
{
	rect.adjust(-(width/2 - rect.width()/2), 0, width/2 - rect.width()/2, 0);
}

TextPointItem::TextPointItem(const QPoint &point, const QString *text,
  const QFont *font, const QImage *img, const QColor *color,
  const QColor *bgColor, bool padding) : TextItem(font ? text : 0),
  _font(font), _img(img), _color(color), _bgColor(bgColor)
{
	if (_text) {
		QFontMetrics fm(*_font);
		int limit = _font->pixelSize() * MAX_TEXT_WIDTH;
		_textRect = fm.boundingRect(QRect(0, 0, limit, 0), FLAGS, *_text);
		_textRect.adjust(0, 0, 1, 1);

		if (_bgColor && _textRect.width() < _font->pixelSize() * MIN_BOX_WIDTH)
			expand(_textRect, _font->pixelSize() * MIN_BOX_WIDTH);
	}

	setPos(point, padding);
}

void TextPointItem::setPos(const QPoint &point, bool padding)
{
	QPainterPath shape;
	QRect iconRect;

	if (_img) {
		int xOffset = padding ? _img->width() : _img->width() / 2;
		iconRect = QRect(QPoint(point.x() - xOffset, point.y()
		  - _img->height()/2), _img->size());
		_textRect.moveTopLeft(QPoint(point.x() + _img->width()/2, point.y()
		  - _textRect.height()/2));
	} else
		_textRect.moveCenter(point);

	_rect = _textRect | iconRect;
	shape.addRect(_rect);
	_shape = shape;
}

void TextPointItem::paint(QPainter *painter) const
{
	if (_img)
		painter->drawImage(QPoint(_rect.left(), _rect.center().y()
		  - _img->height()/2), *_img);

	if (_text) {
		if (_bgColor) {
			painter->setPen(*_color);
			painter->setBrush(*_bgColor);
			painter->drawRect(_textRect);
			painter->setBrush(Qt::NoBrush);
			painter->setFont(*_font);
			painter->drawText(_textRect, FLAGS, *_text);
		} else {
			QImage img(_textRect.size(), QImage::Format_ARGB32_Premultiplied);
			img.fill(Qt::transparent);
			QPainter ip(&img);
			ip.setPen(Qt::white);
			ip.setFont(*_font);
			ip.drawText(img.rect(), FLAGS, *_text);

			painter->drawImage(_textRect.x() - 1, _textRect.y() - 1, img);
			painter->drawImage(_textRect.x() + 1, _textRect.y() + 1, img);
			painter->drawImage(_textRect.x() - 1, _textRect.y() + 1, img);
			painter->drawImage(_textRect.x() + 1, _textRect.y() - 1, img);
			painter->drawImage(_textRect.x(), _textRect.y() - 1, img);
			painter->drawImage(_textRect.x(), _textRect.y() + 1, img);
			painter->drawImage(_textRect.x() - 1, _textRect.y(), img);
			painter->drawImage(_textRect.x() + 1, _textRect.y(), img);

			if (_color) {
				painter->setFont(*_font);
				painter->setPen(*_color);
				painter->drawText(_textRect, FLAGS, *_text);
			} else {
				img.invertPixels();
				painter->drawImage(_textRect, img);
			}
		}
	}

	//painter->setPen(Qt::red);
	//painter->drawRect(_rect);
}
