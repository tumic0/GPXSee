#include <QFont>
#include <QPainter>
#include "textpathitem.h"


#define MAX_TEXT_ANGLE 30

static bool intersection(const QLineF &line, const QRectF &rect,
  QPointF *p)
{
	if (line.intersect(QLineF(rect.topLeft(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.intersect(QLineF(rect.topLeft(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.intersect(QLineF(rect.bottomRight(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.intersect(QLineF(rect.bottomRight(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		return true;

	return false;
}

static QPainterPath subpath(const QList<QLineF> &lines, int start, int end,
  qreal cut)
{
	qreal ss = 0, es = 0;
	int si = start, ei = end;

	for (int i = start; i <= end; i++) {
		qreal len = lines.at(i).length();
		if (ss + len < cut / 2) {
			ss += len;
			si++;
		} else
			break;
	}
	for (int i = end; i >= start; i--) {
		qreal len = lines.at(i).length();
		if (es + len < cut / 2) {
			es += len;
			ei--;
		} else
			break;
	}

	QLineF sl(lines.at(si).p2(), lines.at(si).p1());
	sl.setLength(sl.length() - (cut / 2 - ss));
	QLineF el(lines.at(ei));
	el.setLength(el.length() - (cut / 2 - es));

	QPainterPath p(sl.p2());
	for (int i = si; i <= ei; i++)
		p.lineTo(lines.at(i).p2());
	p.setElementPositionAt(p.elementCount() - 1, el.p2().x(), el.p2().y());

	return p;
}

static QList<QLineF> lineString(const QPolygonF &path,
  const QRectF &boundingRect)
{
	QList<QLineF> lines;
	int start = 0, end = path.count() - 1;
	QPointF p;

	for (int i = 0; i < path.count(); i++) {
		if (boundingRect.contains(path.at(i))) {
			start = i;
			break;
		}
	}
	for (int i = path.count() - 1; i >= 0; i--) {
		if (boundingRect.contains(path.at(i))) {
			end = i;
			break;
		}
	}

	if (start > 0) {
		QLineF l(path.at(start-1), path.at(start));
		if (intersection(l, boundingRect, &p))
			lines.append(QLineF(p, path.at(start)));
	}
	for (int i = start + 1; i <= end; i++)
		lines.append(QLineF(path.at(i-1), path.at(i)));
	if (end < path.count() - 1) {
		QLineF l(path.at(end), path.at(end+1));
		if (intersection(l, boundingRect, &p))
			lines.append(QLineF(path.at(end), p));
	}

	return lines;
}

static QPainterPath textPath(const QPolygonF &path, qreal textWidth,
  qreal charWidth, const QRectF &tileRect)
{
	QList<QLineF> lines(lineString(path, tileRect));
	if (lines.isEmpty())
		return QPainterPath();
	qreal length = 0;
	qreal angle = lines.first().angle();
	int last = 0;

	for (int i = 0; i < lines.size(); i++) {
		qreal sl = lines.at(i).length();
		qreal a = lines.at(i).angle();

		if (!tileRect.contains(lines.at(i).p2()) || sl < charWidth
		  || qAbs(angle - a) > MAX_TEXT_ANGLE) {
			if (length > textWidth)
				return subpath(lines, last, i - 1, length - textWidth);
			last = i;
			length = 0;
		} else
			length += sl;

		angle = a;
	}

	return (length > textWidth)
	  ? subpath(lines, last, lines.size() - 1, length - textWidth)
	  : QPainterPath();
}

static bool reverse(const QPainterPath &path)
{
	QLineF l(path.elementAt(0), path.elementAt(1));
	qreal angle = l.angle();
	return (angle > 90 && angle < 270) ? true : false;
}


TextPathItem::TextPathItem(const QPolygonF &line, const QString *label,
  const QRect &tileRect, const QFont *font, const QColor *color)
  : TextItem(label), _font(font), _color(color)
{
	qreal cw = font->pixelSize() * 0.7;
	qreal textWidth = _text->size() * cw;
	qreal mw = font->pixelSize() / 2;
	_path = textPath(line, textWidth, cw, tileRect.adjusted(mw, mw, -mw, -mw));
	if (_path.isEmpty())
		return;

	if (reverse(_path))
		_path = _path.toReversed();

	QPainterPathStroker s;
	s.setWidth(font->pixelSize());
	s.setCapStyle(Qt::FlatCap);
	_shape = s.createStroke(_path).simplified();
	_rect = _shape.boundingRect();
}

void TextPathItem::paint(QPainter *painter) const
{
	QFontMetrics fm(*_font);
	int textWidth = fm.width(*_text);

	qreal factor = (textWidth) / qMax(_path.length(), (qreal)textWidth);
	qreal percent = (1.0 - factor) / 2.0;

	QTransform t = painter->transform();

	painter->setFont(*_font);
	painter->setPen(Qt::white);

	for (int i = 0; i < _text->size(); i++) {
		QPointF point = _path.pointAtPercent(percent);
		qreal angle = _path.angleAtPercent(percent);

		painter->translate(point);
		painter->rotate(-angle);
		painter->drawText(QPoint(-1, fm.descent() - 1), _text->at(i));
		painter->drawText(QPoint(1, fm.descent() + 1), _text->at(i));
		painter->drawText(QPoint(-1, fm.descent() + 1), _text->at(i));
		painter->drawText(QPoint(1, fm.descent() -1), _text->at(i));
		painter->drawText(QPoint(0, fm.descent() - 1), _text->at(i));
		painter->drawText(QPoint(0, fm.descent() + 1), _text->at(i));
		painter->drawText(QPoint(-1, fm.descent()), _text->at(i));
		painter->drawText(QPoint(1, fm.descent()), _text->at(i));
		painter->setTransform(t);

		int width = fm.charWidth(*_text, i);
		percent += ((qreal)width / (qreal)textWidth) * factor;
	}
	percent = (1.0 - factor) / 2.0;

	painter->setPen(_color ? *_color : Qt::black);
	for (int i = 0; i < _text->size(); i++) {
		QPointF point = _path.pointAtPercent(percent);
		qreal angle = _path.angleAtPercent(percent);

		painter->translate(point);
		painter->rotate(-angle);
		painter->drawText(QPoint(0, fm.descent()), _text->at(i));
		painter->setTransform(t);

		int width = fm.charWidth(*_text, i);
		percent += ((qreal)width / (qreal)textWidth) * factor;
	}

	//painter->setPen(Qt::red);
	//painter->drawPath(_shape);
}
