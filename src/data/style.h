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
	PointStyle(const QColor &color, int size = -1)
	  : _color(color), _size(size) {}

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
	PolygonStyle() : _width(-1) {}
	PolygonStyle(const QColor &fill, const QColor &stroke = QColor(),
	  qreal width = -1) : _fill(fill), _stroke(stroke), _width(width) {}

	const QColor &fill() const {return _fill;}
	const QColor &stroke() const {return _stroke;}
	qreal width() const {return _width;}

	bool isValid() const
	  {return _fill.isValid() || _stroke.isValid();}

private:
	QColor _fill;
	QColor _stroke;
	qreal _width;
};

class LineStyle {
public:
	LineStyle() : _width(-1) {}
	LineStyle(const QColor &color, qreal width = -1)
	  : _color(color), _width(width) {}

	const QColor &color() const {return _color;}
	qreal width() const {return _width;}

private:
	QColor _color;
	qreal _width;
};

#endif // STYLE_H
