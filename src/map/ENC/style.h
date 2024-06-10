#ifndef ENC_STYLE_H
#define ENC_STYLE_H

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QMap>

namespace ENC {

class Style
{
public:
	enum FontSize {
		None,
		Small,
		Normal,
		Large,
	};

	class Polygon {
	public:
		Polygon() : _brush(Qt::NoBrush), _pen(Qt::NoPen) {}
		Polygon(const QBrush &brush, const QPen &pen = Qt::NoPen)
			: _brush(brush)
		{
			_pen = (pen == Qt::NoPen) ? QPen(_brush, 0) : pen;
		}
		Polygon(const QImage &img)
		  : _brush(Qt::NoBrush), _pen(Qt::NoPen), _img(img.convertToFormat(
		  QImage::Format_ARGB32_Premultiplied)) {}

		const QPen &pen() const {return _pen;}
		const QBrush &brush() const {return _brush;}
		const QImage &img() const {return _img;}

	private:
		QBrush _brush;
		QPen _pen;
		QImage _img;
	};

	class Line {
	public:
		Line() : _pen(Qt::NoPen), _textFontSize(None) {}
		Line(const QPen &pen) : _pen(pen), _textFontSize(None) {}
		Line(const QImage &img)
		  : _pen(Qt::NoPen), _textFontSize(None), _img(img.convertToFormat(
		  QImage::Format_ARGB32_Premultiplied)) {}

		void setTextColor(const QColor &color) {_textColor = color;}
		void setTextFontSize(FontSize size) {_textFontSize = size;}

		const QPen &pen() const {return _pen;}
		const QColor &textColor() const {return _textColor;}
		FontSize textFontSize() const {return _textFontSize;}
		const QImage &img() const {return _img;}

	private:
		QPen _pen;
		QColor _textColor;
		FontSize _textFontSize;
		QImage _img;
	};

	class Point {
	public:
		Point() : _textColor(Qt::black), _haloColor(Qt::white),
		  _textFontSize(Normal) {}
		Point(const QImage &img, FontSize fontSize = Normal,
		  const QPoint &offset = QPoint(0, 0))
		  : _textColor(Qt::black), _haloColor(Qt::white),
		  _textFontSize(fontSize), _img(img), _offset(offset) {}

		void setTextColor(const QColor &color) {_textColor = color;}
		void setHaloColor(const QColor &color) {_haloColor = color;}
		void setTextFontSize(FontSize size) {_textFontSize = size;}

		const QColor &textColor() const {return _textColor;}
		const QColor &haloColor() const {return _haloColor;}
		FontSize textFontSize() const {return _textFontSize;}
		const QImage &img() const {return _img;}
		const QPoint &offset() const {return _offset;}

	private:
		QColor _textColor, _haloColor;
		FontSize _textFontSize;
		QImage _img;
		QPoint _offset;
	};

	Style(qreal ratio);

	const Line &line(uint type) const;
	const Polygon &polygon(uint type) const;
	const Point &point(uint type) const;
	const QVector<uint> &drawOrder() const {return _drawOrder;}

	const QFont *font(Style::FontSize size) const;
	const QImage *light() const {return &_light;}
	const QImage *signal() const {return &_signal;}
	const QPoint &lightOffset() const {return _lightOffset;}
	const QPoint &signalOffset() const {return _signalOffset;}

private:
	void polygonStyle();
	void lineStyle(qreal ratio);
	void pointStyle(qreal ratio);

	QMap<uint, Line> _lines;
	QMap<uint, Polygon> _polygons;
	QMap<uint, Point> _points;
	QVector<uint> _drawOrder;

	/* Fonts and images must be initialized after QGuiApplication! */
	QFont _small, _normal, _large;
	QImage _light, _signal;
	QPoint _lightOffset, _signalOffset;
};

}

#endif // ENC_STYLE_H
