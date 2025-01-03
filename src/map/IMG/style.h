#ifndef IMG_STYLE_H
#define IMG_STYLE_H

#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDebug>
#include "subfile.h"

#define TYPE(t) ((t)<<8)

namespace IMG {

class Style
{
public:
	enum FontSize {
		NotSet = 0,
		None = 1,
		Small = 2,
		Normal = 3,
		Large = 4,
		ExtraSmall = 5
	};

	class Font {
	public:
		Font() :_size(NotSet) {}
		Font(const QColor &color, FontSize size) : _color(color), _size(size) {}

		const QColor &color() const {return _color;}
		FontSize size() const {return _size;}
		void setColor(const QColor &color) {_color = color;}
		void setSize(FontSize size) {_size = size;}

	private:
		QColor _color;
		FontSize _size;
	};

	class Polygon {
	public:
		Polygon() : _brush(Qt::NoBrush), _pen(Qt::NoPen) {}
		Polygon(const QBrush &brush, const QPen &pen = Qt::NoPen)
			: _brush(brush)
		{
			_pen = (pen == Qt::NoPen) ? QPen(_brush, 0) : pen;
		}

		const QPen &pen() const {return _pen;}
		const QBrush &brush() const {return _brush;}

	private:
		QBrush _brush;
		QPen _pen;
	};

	class Line {
	public:
		Line() : _foreground(Qt::NoPen), _background(Qt::NoPen) {}
		Line(const QPen &foreground, const QPen &background = Qt::NoPen)
		  : _foreground(foreground), _background(background) {}
		Line(const QImage &img)
		  : _foreground(Qt::NoPen), _background(Qt::NoPen),
		  _img(img.convertToFormat(QImage::Format_ARGB32_Premultiplied)) {}

		const QPen &foreground() const {return _foreground;}
		const QPen &background() const {return _background;}
		const Font &text() const {return _text;}
		const QImage &img() const {return _img;}

	private:
		friend class Style;

		void setTextColor(const QColor &color) {_text.setColor(color);}
		void setTextFontSize(FontSize size) {_text.setSize(size);}

		QPen _foreground, _background;
		Font _text;
		QImage _img;
	};

	class Point {
	public:
		Point() {}
		Point(FontSize fontSize, const QColor &textColor = QColor())
		  : _text(textColor, fontSize) {}
		Point(const QImage &img, const QPoint &offset = QPoint(0, 0))
		  : _img(img), _offset(offset) {}

		const Font &text() const {return _text;}
		const QImage &img() const {return _img;}
		const QPoint &offset() const {return _offset;}

	private:
		friend class Style;

		void setTextColor(const QColor &color) {_text.setColor(color);}
		void setTextFontSize(FontSize size) {_text.setSize(size);}

		Font _text;
		QImage _img;
		QPoint _offset;
	};


	Style(qreal ratio, SubFile *typ = 0);

	const Line &line(quint32 type) const;
	const Polygon &polygon(quint32 type) const;
	const Point &point(quint32 type) const;
	const QList<quint32> &drawOrder() const {return _drawOrder;}
	const QFont *font(Style::FontSize size, Style::FontSize defaultSize
	  = Style::Normal) const;

	const QImage *light() const {return &_light;}
	const QPoint &lightOffset() const {return _lightOffset;}

	static bool isPOI(quint32 type)
	  {return !((type >= TYPE(0x01) && type <= TYPE(0x1f))
	  || (type >= 0x11400 && type < 0x11500));}
	static bool isContourLine(quint32 type)
	  {return ((type >= TYPE(0x20) && type <= TYPE(0x25))
	  || (type & 0xffff00) == TYPE(0x109));}
	static bool isWaterArea(quint32 type)
	  {return ((type >= TYPE(0x3c) && type <= TYPE(0x44))
	  || (type & 0xffff00) == TYPE(0x10b));}
	static bool isWaterLine(quint32 type)
	  {return (type == TYPE(0x26) || type == TYPE(0x18)
	  || type == TYPE(0x1f));}
	static bool isMilitaryArea(quint32 type)
	  {return (type == TYPE(0x04) || type == 0x10901);}
	static bool isNatureReserve(quint32 type)
	  {return (type == TYPE(0x16) || type == 0x10a03);}
	static bool isSpot(quint32 type)
	  {return (type == TYPE(0x62) || type == TYPE(0x63));}
	static bool isMajorRoad(quint32 type)
	  {return (type <= TYPE(0x04));}
	static bool isCountry(quint32 type)
	  {return (type >= 0x1400 && type <= 0x153f);}
	static bool isState(quint32 type)
	  {return (type == TYPE(0x1e));}
	static bool isRaster(quint32 type)
	  {return (type == 0x10613);}
	static bool isDepthPoint(quint32 type)
	  {return (type == 0x10301);}
	static bool isObstructionPoint(quint32 type)
	  {return (type >= 0x10400 && type <= 0x10401);}
	static bool isBuoy(quint32 type)
	  {return (type >= 0x10200 && type < 0x10300);}
	static bool isLight(quint32 type)
	  {return (type >= 0x10100 && type < 0x10200);}
	static bool isMarinePoint(quint32 type)
	  {return type >= 0x10100 && type < 0x10a00;}
	static bool isMarina(quint32 type)
	  {return type == 0x10703;}

private:
	struct Section {
		quint32 offset;
		quint32 size;
		quint32 arrayOffset;
		quint32 arraySize;
		quint16 arrayItemSize;
	};

	struct ItemInfo {
		quint32 offset;
		quint8 type;
		quint8 subtype;
		bool extended;
	};

	bool parseTYPFile(SubFile *typ);
	bool parsePoints(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool parsePoint(SubFile *file, SubFile::Handle &hdl,
	  const Section &section, const ItemInfo &info, quint32 type);
	bool parseLines(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool parseLine(SubFile *file, SubFile::Handle &hdl,
	  const Section &section, const ItemInfo &info, quint32 type);
	bool parsePolygons(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool parsePolygon(SubFile *file, SubFile::Handle &hdl,
	  const Section &section, const ItemInfo &info, quint32 type);
	bool parseDrawOrder(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	void defaultPolygonStyle();
	void defaultLineStyle(qreal ratio);
	void defaultPointStyle(qreal ratio);

	static bool itemInfo(SubFile *file, SubFile::Handle &hdl,
	  const Section &section, ItemInfo &info);

	QMap<quint32, Line> _lines;
	QMap<quint32, Polygon> _polygons;
	QMap<quint32, Point> _points;
	QList<quint32> _drawOrder;

	/* Fonts and images must be initialized after QGuiApplication! */
	QFont _large, _normal, _small, _extraSmall;

	QImage _light;
	QPoint _lightOffset;
};

}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const IMG::Style::Font &font);
QDebug operator<<(QDebug dbg, const IMG::Style::Polygon &polygon);
QDebug operator<<(QDebug dbg, const IMG::Style::Line &line);
QDebug operator<<(QDebug dbg, const IMG::Style::Point &point);
#endif // QT_NO_DEBUG

#endif // IMG_STYLE_H
