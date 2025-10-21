#include <cmath>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QStaticText>
#include "textpointitem.h"

#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)
#define MIN_BOX_WIDTH 2

static void expand(QRectF &rect, int width)
{
	rect.adjust(-(width/2 - rect.width()/2), 0, width/2 - rect.width()/2, 0);
}

TextPointItem::TextPointItem(const QPoint &point, const QString *text,
  const QFont *font, const QImage *img, const QColor *color,
  const QColor *haloColor, const QColor *bgColor, int padding, double rotate,
  Anchor textAnchor, int maxWidth)
  : TextItem(font ? text : 0), _font(font), _img(img), _color(color),
  _haloColor(haloColor), _bgColor(bgColor), _rotate(rotate), _anchor(textAnchor)
{
	if (_text) {
		QFontMetrics fm(*_font);
		int limit = _font->pixelSize() * maxWidth;
		_textRect = fm.boundingRect(QRect(0, 0, limit, 0), FLAGS, *_text);
		_textRect.adjust(-1, 0, 2, 0);

		if (_bgColor && _textRect.width() < _font->pixelSize() * MIN_BOX_WIDTH)
			expand(_textRect, _font->pixelSize() * MIN_BOX_WIDTH);
	}

	setPos(point, padding);
}

void TextPointItem::moveTextRect(const QPoint &pos, const QRectF &iconRect,
  int padding)
{
	switch (_anchor) {
		case Center:
			_textRect.moveCenter(pos);
			break;
		case Left:
			_textRect.moveTopLeft(pos - QPointF(-iconRect.width() / 2
			  - padding, _textRect.height() / 2));
			break;
		case Right:
			_textRect.moveTopRight(pos - QPointF(iconRect.width() / 2
			  + padding, _textRect.height() / 2));
			break;
		case Bottom:
			_textRect.moveBottomLeft(pos - QPointF(_textRect.width() / 2,
			  iconRect.height() / 2));
			break;
		case Top:
			_textRect.moveTopLeft(pos - QPointF(_textRect.width() / 2,
			  -iconRect.height() / 2));
			break;
	}
}

void TextPointItem::setPos(const QPoint &point, int padding)
{
	QPainterPath shape;
	QRectF iconRect;

	if (_img && !_img->isNull()) {
		QSizeF s(_img->size() / _img->devicePixelRatioF());
		iconRect = QRectF(QPointF(point.x() - s.width() / 2,
		  point.y() - s.height()/2), s);
		moveTextRect(point, iconRect, padding);
	} else
		_textRect.moveCenter(point);

	_rect = _textRect | iconRect;
	shape.addRect(_rect);
	_shape = shape;
	_pos = point;
}

void TextPointItem::paint(QPainter *painter) const
{
	if (_img && !_img->isNull()) {
		QSizeF s(_img->size() / _img->devicePixelRatioF());
		if (std::isnan(_rotate))
			painter->drawImage(QPointF(_pos.x() - s.width()/2,
			  _pos.y() - s.height()/2), *_img);
		else {
			painter->save();
			painter->translate(QPointF(_pos.x() /*+ s.width()/2*/, _pos.y()));
			painter->rotate(_rotate);
			painter->drawImage(QPointF(-s.width()/2, -s.height()/2), *_img);
			painter->restore();
		}
	}

	if (_text && _font && _color) {
		if (_bgColor) {
			painter->setPen(*_color);
			painter->setBrush(*_bgColor);
			painter->drawRect(_textRect);
			painter->setBrush(Qt::NoBrush);
			painter->setFont(*_font);
			painter->drawText(_textRect, FLAGS, *_text);
		} else if (_haloColor) {
			QStaticText st(*_text);
			st.setTextFormat(Qt::PlainText);
			st.setTextWidth(_textRect.width());
			st.setTextOption(QTextOption(Qt::AlignHCenter));
			st.setPerformanceHint(QStaticText::AggressiveCaching);

			painter->setPen(*_haloColor);
			painter->setFont(*_font);

			painter->drawStaticText(_textRect.topLeft() + QPointF(-1, -1), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(+1, +1), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(-1, +1), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(+1, -1), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(0, -1), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(0, +1), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(-1, 0), st);
			painter->drawStaticText(_textRect.topLeft() + QPointF(+1, 0), st);

			painter->setPen(*_color);
			painter->drawStaticText(_textRect.topLeft(), st);
		} else {
			painter->setPen(*_color);
			painter->setFont(*_font);
			painter->drawText(_textRect, FLAGS, *_text);
		}
	}

	//painter->setPen(Qt::red);
	//painter->setBrush(Qt::NoBrush);
	//painter->setRenderHint(QPainter::Antialiasing, false);
	//painter->drawRect(_rect);
}
