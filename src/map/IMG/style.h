#ifndef STYLE_H
#define STYLE_H

#include <QPen>
#include <QBrush>
#include <QDebug>
#include "subfile.h"

#define TYPE(t) ((t)<<8)

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

	enum POIClass {
		Unknown,
		Food,
		Accommodation,
		Recreation,
		Shopping,
		Transport,
		Services,
		Community,
		Elementary,
		ManmadePlaces,
		NaturePlaces
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
		QColor _textColor;
	};

	class Line {
	public:
		Line() : _foreground(Qt::NoPen), _background(Qt::NoPen),
		  _textFontSize(NotSet) {}
		Line(const QPen &foreground, const QPen &background = Qt::NoPen)
		  : _foreground(foreground), _background(background),
		  _textFontSize(NotSet) {}
		Line(const QImage &img)
		  : _foreground(Qt::NoPen), _background(Qt::NoPen),
		  _textFontSize(NotSet), _img(img.convertToFormat(
		  QImage::Format_ARGB32_Premultiplied)) {}

		void setTextColor(const QColor &color) {_textColor = color;}
		void setTextFontSize(FontSize size) {_textFontSize = size;}

		const QPen &foreground() const {return _foreground;}
		const QPen &background() const {return _background;}
		const QColor &textColor() const {return _textColor;}
		FontSize textFontSize() const {return _textFontSize;}
		const QImage &img() const {return _img;}

	private:
		QPen _foreground, _background;
		QColor _textColor;
		FontSize _textFontSize;
		QImage _img;
	};

	class Point {
	public:
		Point() : _textFontSize(NotSet) {}
		Point(FontSize fontSize, const QColor &textColor = QColor())
		  : _textColor(textColor), _textFontSize(fontSize) {}
		Point(const QImage &img) : _textFontSize(NotSet), _img(img) {}

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


	Style(SubFile *typ = 0);

	const Line &line(quint32 type) const;
	const Polygon &polygon(quint32 type) const;
	const Point &point(quint32 type) const;
	const QList<quint32> &drawOrder() const {return _drawOrder;}

	static bool isPOI(quint32 type)
	  {return !((type >= TYPE(0x01) && type <= TYPE(0x1f))
	  || (type >= 0x11400 && type < 0x11500));}
	static bool isContourLine(quint32 type)
	  {return ((type >= TYPE(0x20) && type <= TYPE(0x25))
	  || (type & 0xffff00) == TYPE(0x109));}
	static bool isWaterArea(quint32 type)
	  {return ((type >= TYPE(0x3c) && type <= TYPE(0x44))
	  || (type & 0xffff00) == TYPE(0x10b));}
	static bool isMilitaryArea(quint32 type)
	  {return (type == TYPE(0x04) || type == 0x10901);}
	static bool isNatureReserve(quint32 type)
	  {return (type == TYPE(0x16) || type == 0x10a03);}
	static bool isSpot(quint32 type)
	  {return (type == TYPE(0x62) || type == TYPE(0x63));}
	static bool isSummit(quint32 type)
	  {return (type == 0x6616);}
	static bool isMajorRoad(quint32 type)
	  {return (type <= TYPE(0x04));}

	static POIClass poiClass(quint32 type);

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

	bool parseTYPFile(SubFile *file);
	bool parsePoints(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool parseLines(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool parsePolygons(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool parseDrawOrder(SubFile *file, SubFile::Handle &hdl,
	  const Section &section);
	bool itemInfo(SubFile *file, SubFile::Handle &hdl,
	  const Section &section, ItemInfo &info);
	void defaultPolygonStyle();
	void defaultLineStyle();
	void defaultPointStyle();

	QMap<quint32, Line> _lines;
	QMap<quint32, Polygon> _polygons;
	QMap<quint32, Point> _points;
	QList<quint32> _drawOrder;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Style::Polygon &polygon);
QDebug operator<<(QDebug dbg, const Style::Line &line);
QDebug operator<<(QDebug dbg, const Style::Point &point);
#endif // QT_NO_DEBUG

#endif // STYLE_H
