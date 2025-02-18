#include <QFont>
#include <QPainter>
#include "textpathitem.h"

#define CHAR_RATIO     0.55
#define MAX_ANGLE      30
#define PADDING        2

static void swap(const QLineF &line, QPointF *p1, QPointF *p2)
{
	QPointF lp1(line.p1());
	QPointF lp2(line.p2());

	if ((lp1.rx() < lp2.rx() && p1->rx() > p2->rx())
	  || (lp1.ry() < lp2.ry() && p1->ry() > p2->ry())
	  || (lp1.rx() > lp2.rx() && p1->rx() < p2->rx())
	  || (lp1.ry() > lp2.ry() && p1->ry() < p2->ry())) {
		QPointF tmp(*p2);
		*p2 = *p1;
		*p1 = tmp;
	}
}

static bool intersection(const QLineF &line, const QRectF &rect, QPointF *p1,
  QPointF *p2)
{
	QPointF *p = p1;

	if (line.intersects(QLineF(rect.topLeft(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		p = p2;
	if (line.intersects(QLineF(rect.topLeft(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2) {
			swap(line, p1, p2);
			return true;
		}
		p = p2;
	}
	if (line.intersects(QLineF(rect.bottomRight(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2) {
			swap(line, p1, p2);
			return true;
		}
		p = p2;
	}
	if (line.intersects(QLineF(rect.bottomRight(), rect.topRight()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2) {
			swap(line, p1, p2);
			return true;
		}
	}

	Q_ASSERT(p != p2);

	return false;
}

static bool intersection(const QLineF &line, const QRectF &rect, QPointF *p)
{
	if (line.intersects(QLineF(rect.topLeft(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.intersects(QLineF(rect.topLeft(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.intersects(QLineF(rect.bottomRight(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.intersects(QLineF(rect.bottomRight(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		return true;

	return false;
}

static QPainterPath subpath(const QPolygonF &path, int start, int end,
  qreal cut)
{
	qreal ss = 0, es = 0;
	int si = start, ei = end;

	for (int i = start; i < end; i++) {
		QLineF l(path.at(i), path.at(i+1));
		qreal len = l.length();
		if (ss + len < cut / 2) {
			ss += len;
			si++;
		} else
			break;
	}
	for (int i = end; i > start; i--) {
		QLineF l(path.at(i), path.at(i-1));
		qreal len = l.length();
		if (es + len < cut / 2) {
			es += len;
			ei--;
		} else
			break;
	}

	QLineF sl(path.at(si+1), path.at(si));
	sl.setLength(sl.length() - (cut / 2 - ss));
	QLineF el(path.at(ei-1), path.at(ei));
	el.setLength(el.length() - (cut / 2 - es));

	QPainterPath p(sl.p2());
	for (int i = si + 1; i < ei; i++)
		p.lineTo(path.at(i));
	p.lineTo(el.p2());

	return p;
}

static QList<QPolygonF> polyLines(const QPolygonF &path, const QRectF &rect)
{
	QList<QPolygonF> lines;
	QPolygonF line;
	bool lastIn = rect.contains(path.first());

	for (int i = 1; i < path.size(); i++) {
		if (rect.contains(path.at(i))) {
			if (lastIn) {
				if (line.isEmpty())
					line.append(path.at(i-1));
				line.append(path.at(i));
			} else {
				QPointF p;
				QLineF l(path.at(i-1), path.at(i));

				if (intersection(l, rect, &p))
					line.append(p);
				line.append(path.at(i));
			}

			lastIn = true;
		} else {
			QLineF l(path.at(i-1), path.at(i));

			if (lastIn) {
				QPointF p;
				if (line.isEmpty())
					line.append(path.at(i-1));
				if (intersection(l, rect, &p))
					line.append(p);
				lines.append(line);
				line.clear();
			} else {
				QPointF p1, p2;
				if (intersection(l, rect, &p1, &p2)) {
					line.append(p1);
					line.append(p2);
					lines.append(line);
					line.clear();
				}
			}

			lastIn = false;
		}
	}

	if (!line.isEmpty())
		lines.append(line);

	return lines;
}

static QList<QPolygonF> polyLines(const QPainterPath &path, const QRectF &rect)
{
	QList<QPolygonF> lines;
	QPolygonF line;
	bool lastIn = rect.contains(path.elementAt(0));

	for (int i = 1; i < path.elementCount(); i++) {
		if (rect.contains(path.elementAt(i))) {
			if (lastIn) {
				if (line.isEmpty())
					line.append(path.elementAt(i-1));
				line.append(path.elementAt(i));
			} else {
				QPointF p;
				QLineF l(path.elementAt(i-1), path.elementAt(i));

				if (intersection(l, rect, &p))
					line.append(p);
				line.append(path.elementAt(i));
			}

			lastIn = true;
		} else {
			QLineF l(path.elementAt(i-1), path.elementAt(i));

			if (lastIn) {
				QPointF p;
				if (line.isEmpty())
					line.append(path.elementAt(i-1));
				if (intersection(l, rect, &p))
					line.append(p);
				lines.append(line);
				line.clear();
			} else {
				QPointF p1, p2;
				if (intersection(l, rect, &p1, &p2)) {
					line.append(p1);
					line.append(p2);
					lines.append(line);
					line.clear();
				}
			}

			lastIn = false;
		}
	}

	if (!line.isEmpty())
		lines.append(line);

	return lines;
}

static bool reverse(const QPainterPath &path)
{
	QLineF l(path.elementAt(0), path.elementAt(1));
	qreal angle = l.angle();
	return (angle > 90 && angle < 270) ? true : false;
}

static qreal diff(qreal a1, qreal a2)
{
	qreal d = qAbs(a1 - a2);
	return (d > 180) ? 360 - d : d;
}

template<class T>
static QPainterPath textPath(const T &path, qreal textWidth, qreal charWidth,
  const QRectF &tileRect)
{
	if (path.isEmpty())
		return QPainterPath();

	QList<QPolygonF> lines(polyLines(path, tileRect));

	for (int i = 0; i < lines.size(); i++) {
		const QPolygonF &pl = lines.at(i);
		qreal angle = 0, length = 0;
		int last = 0;

		for (int j = 1; j < pl.size(); j ++) {
			QLineF l(pl.at(j-1), pl.at(j));
			qreal sl = l.length();
			qreal a = l.angle();

			if (sl < charWidth) {
				if (length > textWidth)
					return subpath(pl, last, j - 1, length - textWidth);
				last = j;
				length = 0;
			} else if (j > 1 && diff(angle, a) > MAX_ANGLE) {
				if (length > textWidth)
					return subpath(pl, last, j - 1, length - textWidth);
				last = j - 1;
				length = sl;
			} else
				length += sl;

			angle = a;
		}

		if (length > textWidth)
			return subpath(pl, last, pl.size() - 1, length - textWidth);
	}

	return QPainterPath();
}

template<class T>
void TextPathItem::init(const T &line, const QRect &tileRect)
{
	qreal cw, mw, textWidth;
	bool label = _text && _font;

	Q_ASSERT(label || _img);

	if (label && _img) {
		cw = _font->pixelSize() * CHAR_RATIO;
		mw = _font->pixelSize() / 2.0;
		textWidth = _text->size() * cw
		  + (_img->width() / _img->devicePixelRatioF()) + PADDING;
	} else if (label) {
		cw = _font->pixelSize() * CHAR_RATIO;
		mw = _font->pixelSize() / 2.0;
		textWidth = _text->size() * cw;
	} else {
		cw = _img->width() / _img->devicePixelRatioF();
		mw = _img->height() / _img->devicePixelRatioF() / 2.0;
		textWidth = _img->width() / _img->devicePixelRatioF();
	}

	_path = textPath(line, textWidth, cw, tileRect.adjusted(mw, mw, -mw, -mw));
	if (_path.isEmpty())
		return;

	if (reverse(_path)) {
		_path = _path.toReversed();
		_reverse = true;
	}

	QPainterPathStroker s;
	s.setWidth(mw * 2);
	s.setCapStyle(Qt::FlatCap);
	_shape = s.createStroke(_path).simplified();
	_rect = _shape.boundingRect();
}

TextPathItem::TextPathItem(const QPolygonF &line, const QString *label,
  const QRect &tileRect, const QFont *font, const QColor *color,
  const QColor *haloColor, const QImage *img, bool rotate)
  : TextItem(label), _font(font), _color(color), _haloColor(haloColor),
  _img(img), _rotate(rotate), _reverse(false)
{
	init(line, tileRect);
}

TextPathItem::TextPathItem(const QPainterPath &line, const QString *label,
  const QRect &tileRect, const QFont *font, const QColor *color,
  const QColor *haloColor, const QImage *img, bool rotate)
  : TextItem(label), _font(font), _color(color), _haloColor(haloColor),
	_img(img), _rotate(rotate), _reverse(false)
{
	init(line, tileRect);
}

void TextPathItem::paint(QPainter *painter) const
{
	if (_img) {
		QSizeF s(_img->size() / _img->devicePixelRatioF());

		painter->save();
		painter->translate(QPointF(_path.elementAt(0).x, _path.elementAt(0).y));
		painter->rotate(360 - _path.angleAtPercent(0));
		if (_reverse && _rotate) {
			painter->rotate(180);
			painter->translate(-s.width(), 0);
		}
		painter->drawImage(QPointF(0, -s.height()/2), *_img);
		painter->restore();
	}

	if (_text && _font && _color) {
		QFontMetrics fm(*_font);
		int textWidth = fm.boundingRect(*_text).width();
		int imgWidth = _img
		  ? (_img->width() / _img->devicePixelRatioF()) + PADDING : 0;
		qreal imgPercent = imgWidth / _path.length();
		qreal factor = textWidth / qMax(_path.length(), (qreal)(textWidth));
		qreal percent = ((1.0 - factor) + imgPercent) / 2.0;
		QTransform t = painter->transform();

		painter->setFont(*_font);

		if (_haloColor) {
			painter->setPen(*_haloColor);

			for (int i = 0; i < _text->size(); i++) {
				QPointF point = _path.pointAtPercent(percent);
				qreal angle = _path.angleAtPercent(percent);
				QChar c = _text->at(i);

				painter->translate(point);
				painter->rotate(-angle);
				painter->drawText(QPoint(-1, fm.descent() - 1), c);
				painter->drawText(QPoint(1, fm.descent() + 1), c);
				painter->drawText(QPoint(-1, fm.descent() + 1), c);
				painter->drawText(QPoint(1, fm.descent() -1), c);
				painter->drawText(QPoint(0, fm.descent() - 1), c);
				painter->drawText(QPoint(0, fm.descent() + 1), c);
				painter->drawText(QPoint(-1, fm.descent()), c);
				painter->drawText(QPoint(1, fm.descent()), c);
				painter->setTransform(t);

				int width = fm.horizontalAdvance(_text->at(i));
				percent += ((qreal)width / (qreal)textWidth) * factor;
			}
			percent = ((1.0 - factor) + imgPercent) / 2.0;
		}

		painter->setPen(*_color);
		for (int i = 0; i < _text->size(); i++) {
			QPointF point = _path.pointAtPercent(percent);
			qreal angle = _path.angleAtPercent(percent);

			painter->translate(point);
			painter->rotate(-angle);
			painter->drawText(QPoint(0, fm.descent()), _text->at(i));
			painter->setTransform(t);

			int width = fm.horizontalAdvance(_text->at(i));
			percent += ((qreal)width / (qreal)textWidth) * factor;
		}
	}

	//painter->setBrush(Qt::NoBrush);
	//painter->setPen(Qt::red);
	//painter->setRenderHint(QPainter::Antialiasing, false);
	//painter->drawPath(_shape);
}
