#ifndef STYLE_H
#define STYLE_H

#include <QPen>
#include <QBrush>
#include <QPixmap>

class PointStyle {
public:
	PointStyle() : _size(-1) {}
	PointStyle(const QPixmap &icon, const QColor &color = QColor(), int size = -1)
	  : _icon(icon), _color(color), _size(size) {}

	const QColor &color() const {return _color;}
	const QPixmap &icon() const {return _icon;}
	int size() const {return _size;}

private:
	QPixmap _icon;
	QColor _color;
	int _size;
};

class PolygonStyle {
public:
	PolygonStyle()
	  : _pen(QPen(Qt::NoPen)), _brush(QBrush(Qt::NoBrush)) {}
	PolygonStyle(const QPen &pen, const QBrush &brush)
	  : _pen(pen), _brush(brush) {}

	const QPen &pen() const {return _pen;}
	const QBrush &brush() const {return _brush;}

	bool isValid() const
	  {return _pen.style() != Qt::NoPen || _brush.style() != Qt::NoBrush;}

private:
	QPen _pen;
	QBrush _brush;
};

class LineStyle {
public:
	LineStyle() : _width(-1) {}
	LineStyle(const QColor &color, int width = -1)
	  : _color(color), _width(width) {}

	const QColor &color() const {return _color;}
	int width() const {return _width;}

private:
	QColor _color;
	int _width;
};

#endif // STYLE_H
