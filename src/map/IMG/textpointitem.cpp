#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)
#define MAX_TEXT_WIDTH 8

TextPointItem::TextPointItem(const QPoint &point, const QString *text,
  const QFont *font, const QImage *img, const QColor *color)
  : _text(text), _font(font), _img(img), _color(color)
{
	QRect iconRect;

	if (text) {
		QFontMetrics fm(*font);
		int limit = font->pixelSize() * MAX_TEXT_WIDTH;
		_textRect = fm.boundingRect(QRect(0, 0, limit, 0), FLAGS, *text);
		_textRect.adjust(0, 0, 1, 1);
	}
	if (img) {
		iconRect = QRect(QPoint(point.x() - img->width()/2, point.y()
		  - img->height()/2), img->size());
		_textRect.moveTopLeft(QPoint(point.x() + img->width(), point.y()
		  - _textRect.height()/2));
	} else
		_textRect.moveCenter(point);

	_rect = _textRect | iconRect;
	_shape.addRect(_rect);
}

void TextPointItem::paint(QPainter *painter) const
{
	if (_img)
		painter->drawImage(QPoint(_rect.left(), _rect.center().y()
		  - _img->height()/2), *_img);

	if (_text) {
		QImage img(_textRect.size(), QImage::Format_ARGB32_Premultiplied);
		img.fill(Qt::transparent);
		QPainter ip(&img);
		ip.setPen(Qt::white);
		ip.setFont(*_font);
		ip.drawText(img.rect(), FLAGS, *_text);

		painter->drawImage(_textRect.x() - 1, _textRect.y() - 1, img);
		painter->drawImage(_textRect.x() + 1, _textRect.y() + 1, img);
		painter->drawImage(_textRect.x() - 1, _textRect.y() + 1, img);
		painter->drawImage(_textRect.x(), _textRect.y() - 1, img);
		painter->drawImage(_textRect.x(), _textRect.y() + 1, img);
		painter->drawImage(_textRect.x() - 1, _textRect.y(), img);
		painter->drawImage(_textRect.x() + 1, _textRect.y(), img);

		if (_color) {
			painter->setFont(*_font);
			painter->setPen(*_color);
			painter->drawText(_textRect, FLAGS, *_text);
		} else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
			img.invertPixels();
			painter->drawImage(_textRect, img);
#else // QT >= 5.4
			QImage iimg(img.convertToFormat(QImage::Format_ARGB32));
			iimg.invertPixels();
			painter->drawImage(_textRect, iimg);
#endif // QT >= 5.4
		}
	}

	//painter->setPen(Qt::red);
	//painter->drawRect(_rect);
}
