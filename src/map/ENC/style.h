#ifndef ENC_STYLE_H
#define ENC_STYLE_H

#include <QPen>
#include <QBrush>
#include <QMap>
#include "objects.h"

namespace ENC {

#define TYPE(t) ((t)<<16)
#define SUBTYPE(t, s) (((t)<<16)|(s))

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
		Point() : _textColor(Qt::black), _textFontSize(Normal) {}
		Point(const QImage &img, FontSize fontSize = Normal)
		  : _textColor(Qt::black), _textFontSize(fontSize), _img(img) {}

		void setTextColor(const QColor &color) {_textColor = color;}
		void setTextFontSize(FontSize size) {_textFontSize = size;}

		const QColor &textColor() const {return _textColor;}
		FontSize textFontSize() const {return _textFontSize;}
		const QImage &img() const {return _img;}

	private:
		QColor _textColor;
		FontSize _textFontSize;
		QImage _img;
	};

	Style();

	const Line &line(uint type) const;
	const Polygon &polygon(uint type) const;
	const Point &point(uint type) const;
	const QVector<uint> &drawOrder() const {return _drawOrder;}

	static bool isSounding(uint type)
	  {return type == TYPE(SOUNDG);}
	static bool isDistanceMark(uint type)
	  {return (type & 0xFFFF0000) == TYPE(I_DISMAR);}

private:
	void polygonStyle();
	void lineStyle();
	void pointStyle();

	QMap<uint, Line> _lines;
	QMap<uint, Polygon> _polygons;
	QMap<uint, Point> _points;
	QVector<uint> _drawOrder;
};

}

#endif // ENC_STYLE_H
