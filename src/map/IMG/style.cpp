#include <QImage>
#include <QPainter>
#include "style.h"

using namespace IMG;

void Style::defaultPolygonStyle()
{
	_polygons[TYPE(0x01)] = Polygon(QBrush("#dfd3b5"));
	_polygons[TYPE(0x02)] = Polygon(QBrush("#dfd3b5"));
	_polygons[TYPE(0x03)] = Polygon(QBrush("#dfd3b5"));
	_polygons[TYPE(0x04)] = Polygon(QBrush("#ff4040", Qt::BDiagPattern));
	_polygons[TYPE(0x05)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x06)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x07)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x08)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x09)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0a)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0b)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0c)] = Polygon(QBrush("#d6d4ce"));
	_polygons[TYPE(0x0d)] = Polygon(QBrush("#f8e3be"));
	_polygons[TYPE(0x0e)] = Polygon(QBrush("#ffffff"));
	_polygons[TYPE(0x0f)] = Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x10)] = Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x11)] = Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x12)] = Polygon(QBrush("#e6e2d9"));
	_polygons[TYPE(0x13)] = Polygon(QBrush("#dbd0b6"),
	  QPen(QColor("#cdccc4"), 1));
	_polygons[TYPE(0x14)] = Polygon(QBrush("#cadfaf"));
	_polygons[TYPE(0x15)] = Polygon(QBrush("#cadfaf"));
	_polygons[TYPE(0x16)] = Polygon(QBrush(QColor("#9ac269"),
	  Qt::BDiagPattern));
	_polygons[TYPE(0x17)] = Polygon(QBrush("#e4efcf"));
	_polygons[TYPE(0x18)] = Polygon(QBrush("#e3edc6"));
	_polygons[TYPE(0x19)] = Polygon(QBrush("#e3edc6"), QPen("#c9d3a5"));
	_polygons[TYPE(0x1a)] = Polygon(QBrush("#000000", Qt::Dense6Pattern),
	  QPen(QColor("#cdccc4"), 1));
	_polygons[TYPE(0x1e)] = Polygon(QBrush(QColor("#9ac269"),
	  Qt::BDiagPattern));
	_polygons[TYPE(0x1f)] = Polygon(QBrush(QColor("#9ac269"),
	  Qt::BDiagPattern));
	_polygons[TYPE(0x28)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x32)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3c)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3d)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3e)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x3f)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x40)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x41)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x42)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x43)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x44)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x46)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x47)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x48)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x49)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(0x4a)] = Polygon(QBrush("#f1f0e5"), QPen("#f1f0e5"));
	_polygons[TYPE(0x4b)] = Polygon(QBrush("#f1f0e5"), QPen("#f1f0e5"));
	_polygons[TYPE(0x4c)] = Polygon(QBrush("#9fc4e1", Qt::Dense6Pattern));
	_polygons[TYPE(0x4d)] = Polygon(QBrush("#ddf1fd"));
	_polygons[TYPE(0x4e)] = Polygon(QBrush("#f8f8f8"));
	_polygons[TYPE(0x4f)] = Polygon(QBrush("#e4efcf"));
	_polygons[TYPE(0x50)] = Polygon(QBrush("#cadfaf"));
	_polygons[TYPE(0x51)] = Polygon(QBrush("#9fc4e1", Qt::Dense4Pattern));
	_polygons[TYPE(0x52)] = Polygon(QBrush("#cadfaf"));

	// NT types
	_polygons[0x10800] = _polygons[TYPE(0x01)];
	_polygons[0x10801] = _polygons[TYPE(0x02)];
	_polygons[0x10802] = _polygons[TYPE(0x03)];
	_polygons[0x10901] = _polygons[TYPE(0x04)];
	_polygons[0x10902] = _polygons[TYPE(0x05)];
	_polygons[0x10903] = _polygons[TYPE(0x06)];
	_polygons[0x10904] = _polygons[TYPE(0x07)];
	_polygons[0x10905] = _polygons[TYPE(0x08)];
	_polygons[0x10906] = _polygons[TYPE(0x09)];
	_polygons[0x10907] = _polygons[TYPE(0x0a)];
	_polygons[0x10908] = _polygons[TYPE(0x0b)];
	_polygons[0x10909] = _polygons[TYPE(0x0c)];
	_polygons[0x1090a] = _polygons[TYPE(0x0d)];
	_polygons[0x1090b] = _polygons[TYPE(0x0e)];
	_polygons[0x10900] = _polygons[TYPE(0x13)];
	_polygons[0x10a01] = _polygons[TYPE(0x14)];
	_polygons[0x10a02] = _polygons[TYPE(0x15)];
	_polygons[0x10a03] = _polygons[TYPE(0x16)];
	_polygons[0x10a04] = _polygons[TYPE(0x17)];
	_polygons[0x1090c] = _polygons[TYPE(0x18)];
	_polygons[0x1090d] = _polygons[TYPE(0x19)];
	_polygons[0x1090e] = _polygons[TYPE(0x1a)];
	_polygons[0x10a05] = _polygons[TYPE(0x1e)];
	_polygons[0x10a06] = _polygons[TYPE(0x1f)];
	_polygons[0x10b01] = _polygons[TYPE(0x28)];
	_polygons[0x10b02] = _polygons[TYPE(0x32)];
	_polygons[0x10b03] = _polygons[TYPE(0x3c)];
	_polygons[0x10b04] = _polygons[TYPE(0x3d)];
	_polygons[0x10b05] = _polygons[TYPE(0x3e)];
	_polygons[0x10b06] = _polygons[TYPE(0x3f)];
	_polygons[0x10b07] = _polygons[TYPE(0x40)];
	_polygons[0x10b08] = _polygons[TYPE(0x41)];
	_polygons[0x10b09] = _polygons[TYPE(0x42)];
	_polygons[0x10b0a] = _polygons[TYPE(0x43)];
	_polygons[0x10b0b] = _polygons[TYPE(0x44)];
	_polygons[0x10b0c] = _polygons[TYPE(0x46)];
	_polygons[0x10b0d] = _polygons[TYPE(0x47)];
	_polygons[0x10b0e] = _polygons[TYPE(0x48)];
	_polygons[0x10b0f] = _polygons[TYPE(0x49)];
	_polygons[0x10d01] = _polygons[TYPE(0x4b)];
	_polygons[0x10b10] = _polygons[TYPE(0x4c)];
	_polygons[0x10c00] = _polygons[TYPE(0x4d)];
	_polygons[0x10c01] = _polygons[TYPE(0x4e)];
	_polygons[0x10c02] = _polygons[TYPE(0x4f)];
	_polygons[0x10c03] = _polygons[TYPE(0x50)];
	_polygons[0x10c04] = _polygons[TYPE(0x51)];
	_polygons[0x10c05] = _polygons[TYPE(0x52)];

	// Marine stuff
	_polygons[0x10101] = Polygon(QBrush("#e8e064"));
	_polygons[0x10102] = Polygon(QBrush("#d98b21"));
	_polygons[0x10104] = Polygon(QBrush("#ffffff"));
	_polygons[0x10105] = Polygon(QBrush("#a58140"));
	_polygons[0x10301] = Polygon(QBrush("#98c064"));
	_polygons[0x10302] = Polygon(QBrush("#a0a0ff"));
	_polygons[0x10303] = Polygon(QBrush("#b0b0ff"));
	_polygons[0x10304] = Polygon(QBrush("#c0c0ff"));
	_polygons[0x10409] = Polygon(QBrush("#ff4040", Qt::FDiagPattern));
	_polygons[0x10503] = Polygon(QBrush("#ff4040", Qt::FDiagPattern));
	_polygons[0x10601] = Polygon(QBrush("#aaaaaa"));
	_polygons[0x1060a] = Polygon(QBrush("#fcb4fc"));

	// Draw order
	_drawOrder
	  << TYPE(0x4b) << 0x10d01 << 0x10104 << 0x10613 << TYPE(0x4a)
	  << 0x10101 << 0x10102 << 0x10301 << 0x10302 << 0x10303
	  << 0x10304 << 0x10105 << 0x10409 << 0x10503 << 0x10601 << 0x1060a
	  << TYPE(0x01) << 0x10800 << TYPE(0x02) << 0x10801 << TYPE(0x03) << 0x10802
	  << TYPE(0x17) << 0x10a04 << TYPE(0x18) << 0x1090c << TYPE(0x1a) << 0x1090e
	  << TYPE(0x28) << 0x10b01 << TYPE(0x32) << 0x10b02 << TYPE(0x3c) << 0x10b03
	  << TYPE(0x3d) << 0x10b04 << TYPE(0x3e) << 0x10b05 << TYPE(0x3f) << 0x10b06
	  << TYPE(0x40) << 0x10b07 << TYPE(0x41) << 0x10b08 << TYPE(0x42) << 0x10b09
	  << TYPE(0x43) << 0x10b0a << TYPE(0x44) << 0x10b0b << TYPE(0x46) << 0x10b0c
	  << TYPE(0x47) << 0x10b0d << TYPE(0x48) << 0x10b0e << TYPE(0x49) << 0x10b0f
	  << TYPE(0x4c) << 0x10b10 << TYPE(0x4d) << 0x10c00 << TYPE(0x4e) << 0x10c01
	  << TYPE(0x4f) << 0x10c02 << TYPE(0x50) << 0x10c03 << TYPE(0x51) << 0x10c04
	  << TYPE(0x52) << 0x10c05 << TYPE(0x14) << 0x10a01 << TYPE(0x15) << 0x10a02
	  << TYPE(0x16) << 0x10a03 << TYPE(0x1e) << 0x10a05 << TYPE(0x1f) << 0x10a06
	  << TYPE(0x04) << 0x10901 << TYPE(0x05) << 0x10902 << TYPE(0x06) << 0x10903
	  << TYPE(0x07) << 0x10904 << TYPE(0x08) << 0x10905 << TYPE(0x09) << 0x10906
	  << TYPE(0x0a) << 0x10907 << TYPE(0x0b) << 0x10908 << TYPE(0x0c) << 0x10909
	  << TYPE(0x0d) << 0x1090a << TYPE(0x0e) << 0x1090b << TYPE(0x0f)
	  << TYPE(0x10) << TYPE(0x11) << TYPE(0x12) << TYPE(0x19) << 0x1090d
	  << TYPE(0x13) << 0x10900;
}

static QImage railroad()
{
	QImage img(16, 4, QImage::Format_ARGB32_Premultiplied);
	img.fill(QColor("#717171"));
	QPainter p(&img);
	p.setPen(QPen(Qt::white, 2));
	p.drawLine(9, 2, 15, 2);

	return img;
}

void Style::defaultLineStyle()
{
	_lines[TYPE(0x01)] = Line(QPen(QColor("#9bd772"), 2, Qt::SolidLine),
	  QPen(QColor("#72a35a"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x02)] = Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x03)] = Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x04)] = Line(QPen(QColor("#faef75"), 3, Qt::SolidLine),
	  QPen(QColor("#dbd27b"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x05)] = Line(QPen(QColor("#ffffff"), 3, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x06)] = Line(QPen(QColor("#ffffff"), 3, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x07)] = Line(QPen(QColor("#ffffff"), 2, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x08)] = Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x09)] = Line(QPen(QColor("#9bd772"), 2, Qt::SolidLine),
	  QPen(QColor("#72a35a"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x0a)] = Line(QPen(QColor("#aba083"), 1, Qt::DashLine));
	_lines[TYPE(0x0b)] = Line(QPen(QColor("#ffcc78"), 2, Qt::SolidLine),
	  QPen(QColor("#e8a541"), 6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x0c)] = Line(QPen(QColor("#ffffff"), 3, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	_lines[TYPE(0x14)] = Line(railroad());
	_lines[TYPE(0x16)] = Line(QPen(QColor("#aba083"), 1, Qt::DotLine));
	_lines[TYPE(0x18)] = Line(QPen(QColor("#9fc4e1"), 2, Qt::SolidLine));
	_lines[TYPE(0x18)].setTextColor(QColor("#9fc4e1"));
	//_lines[TYPE(0x1a)] = Line(QPen(QColor("#7697b7"), 1, Qt::DashLine));
	_lines[TYPE(0x1b)] = Line(QPen(QColor("#7697b7"), 1, Qt::DashLine));
	_lines[TYPE(0x1c)] = Line(QPen(QColor("#505145"), 1, Qt::DashLine));
	_lines[TYPE(0x1e)] = Line(QPen(QColor("#505145"), 2, Qt::DashDotLine));
	_lines[TYPE(0x1f)] = Line(QPen(QColor("#9fc4e1"), 3, Qt::SolidLine));
	_lines[TYPE(0x1f)].setTextColor(QColor("#9fc4e1"));
	_lines[TYPE(0x20)] = Line(QPen(QColor("#cfcfcf"), 1, Qt::SolidLine));
	_lines[TYPE(0x20)].setTextFontSize(None);
	_lines[TYPE(0x21)] = Line(QPen(QColor("#bfbfbf"), 1, Qt::SolidLine));
	_lines[TYPE(0x21)].setTextColor(QColor("#666666"));
	_lines[TYPE(0x21)].setTextFontSize(Small);
	_lines[TYPE(0x22)] = Line(QPen(QColor("#afafaf"), 1, Qt::SolidLine));
	_lines[TYPE(0x22)].setTextColor(QColor("#666666"));
	_lines[TYPE(0x22)].setTextFontSize(Small);
	_lines[TYPE(0x23)] = Line(QPen(QColor("#55aaff"), 1, Qt::SolidLine));
	_lines[TYPE(0x23)].setTextFontSize(None);
	_lines[TYPE(0x24)] = Line(QPen(QColor("#659aef"), 1, Qt::SolidLine));
	_lines[TYPE(0x24)].setTextColor(QColor("#558adf"));
	_lines[TYPE(0x24)].setTextFontSize(Small);
	_lines[TYPE(0x25)] = Line(QPen(QColor("#558adf"), 1, Qt::SolidLine));
	_lines[TYPE(0x25)].setTextColor(QColor("#558adf"));
	_lines[TYPE(0x25)].setTextFontSize(Small);
	_lines[TYPE(0x26)] = Line(QPen(QColor("#9fc4e1"), 2, Qt::DotLine));
	_lines[TYPE(0x27)] = Line(QPen(QColor("#ffffff"), 4, Qt::SolidLine),
	  QPen(QColor("#d5cdc0"), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	//_lines[TYPE(0x28)] = Line(QPen(QColor("#5a5a5a"), 1, Qt::SolidLine));
	_lines[TYPE(0x29)] = Line(QPen(QColor("#5a5a5a"), 1, Qt::SolidLine));
	_lines[TYPE(0x29)].setTextFontSize(None);

	// NT types
	_lines[0x10c00] = _lines[TYPE(0x14)];
	_lines[0x10a00] = _lines[TYPE(0x18)];
	_lines[0x10b04] = _lines[TYPE(0x1e)];
	_lines[0x10a01] = _lines[TYPE(0x1f)];
	_lines[0x10900] = _lines[TYPE(0x20)];
	_lines[0x10901] = _lines[TYPE(0x21)];
	_lines[0x10902] = _lines[TYPE(0x22)];
	_lines[0x10903] = _lines[TYPE(0x23)];
	_lines[0x10904] = _lines[TYPE(0x24)];
	_lines[0x10905] = _lines[TYPE(0x25)];
	_lines[0x10a02] = _lines[TYPE(0x26)];
	_lines[0x10c02] = _lines[TYPE(0x27)];
	//_lines[0x10c03] = _lines[TYPE(0x28)];
	_lines[0x10c04] = _lines[TYPE(0x29)];

	// Marine stuff
	_lines[0x10101] = Line(QPen(QColor("#000000"), 1, Qt::SolidLine));
	_lines[0x10106] = Line(QImage(":/IMG/cable-line.png"));
	_lines[0x10107] = Line(QPen(QColor("#a58140"), 3, Qt::SolidLine));
	_lines[0x10108] = Line(QPen(QColor("#000000"), 1, Qt::SolidLine));
	_lines[0x10301] = Line(QPen(QColor("#0e1087"), 1, Qt::SolidLine));
	_lines[0x10307] = Line(QPen(QColor("#05620e"), 1, Qt::SolidLine));
	_lines[0x10401] = Line(QImage(":/IMG/cable.png"));
	_lines[0x10402] = Line(QImage(":/IMG/pipeline.png"));
	_lines[0x10405] = Line(QPen(QColor("#e728e7"), 1, Qt::DashLine));
	_lines[0x10409] = Line(QPen(QColor("#000000"), 1, Qt::DotLine));
	_lines[0x10501] = Line(QImage(":/IMG/noanchor-line.png"));
	_lines[0x10503] = Line(QPen(QColor("#e728e7"), 1, Qt::DashLine));
	_lines[0x10507] = Line(QPen(QColor("#e728e7"), 1, Qt::DashLine));
	_lines[0x10601] = Line(QPen(QColor("#000000"), 1, Qt::SolidLine));
	_lines[0x10606] = Line(QImage(":/IMG/anchor-line.png"));
	_lines[0x1060c] = Line(QPen(QColor("#e728e7"), 1, Qt::SolidLine));
	_lines[0x1060d] = Line(QPen(QColor("#eb49eb"), 1, Qt::DashLine));
	_lines[0x10611] = Line(QPen(QColor("#eb49eb"), 1, Qt::DashLine));
}

void Style::defaultPointStyle()
{
	// Countries
	_points[TYPE(0x14)].setTextColor(QColor("#505145"));
	_points[TYPE(0x14)].setTextFontSize(Small);
	_points[TYPE(0x15)].setTextColor(QColor("#505145"));
	_points[TYPE(0x15)].setTextFontSize(Small);

	// Regions
	_points[TYPE(0x1e)].setTextColor(QColor("#505145"));
	_points[TYPE(0x1e)].setTextFontSize(ExtraSmall);
	_points[TYPE(0x28)].setTextFontSize(Small);

	// Cities
	_points[TYPE(0x01)].setTextFontSize(Large);
	_points[TYPE(0x02)].setTextFontSize(Large);
	_points[TYPE(0x03)].setTextFontSize(Large);

	// POI
	_points[0x2a00] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a01] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a02] = Point(QImage(":/IMG/restaurant-noodle-11.png"));
	_points[0x2a03] = Point(QImage(":/IMG/bbq-11.png"));
	_points[0x2a04] = Point(QImage(":/IMG/restaurant-noodle-11.png"));
	_points[0x2a05] = Point(QImage(":/IMG/bakery-11.png"));
	_points[0x2a06] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a07] = Point(QImage(":/IMG/fast-food-11.png"));
	_points[0x2a08] = Point(QImage(":/IMG/restaurant-pizza-11.png"));
	_points[0x2a09] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a0a] = Point(QImage(":/IMG/restaurant-pizza-11.png"));
	_points[0x2a0b] = Point(QImage(":/IMG/restaurant-seafood-11.png"));
	_points[0x2a0c] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a0d] = Point(QImage(":/IMG/bakery-11.png"));
	_points[0x2a0e] = Point(QImage(":/IMG/cafe-11.png"));
	_points[0x2a0f] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a10] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a11] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a12] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x2a13] = Point(QImage(":/IMG/restaurant-11.png"));

	_points[0x2b01] = Point(QImage(":/IMG/lodging-11.png"));
	_points[0x2b02] = Point(QImage(":/IMG/lodging-11.png"));
	_points[0x2b03] = Point(QImage(":/IMG/campsite-11.png"));
	_points[0x2b04] = Point(QImage(":/IMG/village-11.png"));
	_points[0x2b06] = Point(QImage(":/IMG/shelter-11.png"));

	_points[0x2c01] = Point(QImage(":/IMG/amusement-park-11.png"));
	_points[0x2c02] = Point(QImage(":/IMG/museum-11.png"));
	_points[0x2c03] = Point(QImage(":/IMG/library-11.png"));
	_points[0x2c04] = Point(QImage(":/IMG/landmark-11.png"));
	_points[0x2c05] = Point(QImage(":/IMG/school-11.png"));
	_points[0x2c06] = Point(QImage(":/IMG/garden-11.png"));
	_points[0x2c07] = Point(QImage(":/IMG/zoo-11.png"));
	_points[0x2c08] = Point(QImage(":/IMG/soccer-11.png"));
	_points[0x2c0a] = Point(QImage(":/IMG/bar-11.png"));
	_points[0x2c0b] = Point(QImage(":/IMG/place-of-worship-11.png"));
	_points[0x2c0d] = Point(QImage(":/IMG/religious-muslim-11.png"));
	_points[0x2c0e] = Point(QImage(":/IMG/religious-christian-11.png"));
	_points[0x2c10] = Point(QImage(":/IMG/religious-jewish-11.png"));
	_points[0x2d01] = Point(QImage(":/IMG/theatre-11.png"));
	_points[0x2d02] = Point(QImage(":/IMG/bar-11.png"));
	_points[0x2d03] = Point(QImage(":/IMG/cinema-11.png"));
	_points[0x2d04] = Point(QImage(":/IMG/casino-11.png"));
	_points[0x2d05] = Point(QImage(":/IMG/golf-11.png"));
	_points[0x2d06] = Point(QImage(":/IMG/skiing-11.png"));
	_points[0x2d07] = Point(QImage(":/IMG/bowling-alley-11.png"));
	_points[0x2d09] = Point(QImage(":/IMG/swimming-11.png"));
	_points[0x2d0a] = Point(QImage(":/IMG/fitness-centre-11.png"));
	_points[0x2d0b] = Point(QImage(":/IMG/airfield-11.png"));

	_points[0x2e02] = Point(QImage(":/IMG/grocery-11.png"));
	_points[0x2e03] = Point(QImage(":/IMG/shop-11.png"));
	_points[0x2e05] = Point(QImage(":/IMG/pharmacy-11.png"));
	_points[0x2e06] = Point(QImage(":/IMG/convenience-11.png"));
	_points[0x2e07] = Point(QImage(":/IMG/clothing-store-11.png"));
	_points[0x2e08] = Point(QImage(":/IMG/garden-centre-11.png"));
	_points[0x2e09] = Point(QImage(":/IMG/furniture-11.png"));
	_points[0x2e0a] = Point(QImage(":/IMG/shop-11.png"));
	_points[0x2e0c] = Point(QImage(":/IMG/shop-11.png"));

	_points[0x2f01] = Point(QImage(":/IMG/fuel-11.png"));
	_points[0x2f02] = Point(QImage(":/IMG/car-rental-11.png"));
	_points[0x2f03] = Point(QImage(":/IMG/car-repair-11.png"));
	_points[0x2f04] = Point(QImage(":/IMG/airport-11.png"));
	_points[0x2f05] = Point(QImage(":/IMG/post-11.png"));
	_points[0x2f06] = Point(QImage(":/IMG/bank-11.png"));
	_points[0x2f07] = Point(QImage(":/IMG/car-11.png"));
	_points[0x2f08] = Point(QImage(":/IMG/bus-11.png"));
	_points[0x2f09] = Point(QImage(":/IMG/harbor-11.png"));
	_points[0x2f0b] = Point(QImage(":/IMG/parking-11.png"));
	_points[0x2f0b].setTextFontSize(None);
	_points[0x2f0c] = Point(QImage(":/IMG/toilet-11.png"));
	_points[0x2f0c].setTextFontSize(None);
	_points[0x2f10] = Point(QImage(":/IMG/hairdresser-11.png"));
	_points[0x2f12].setTextFontSize(None);
	_points[0x2f13] = Point(QImage(":/IMG/hardware-11.png"));
	_points[0x2f17] = Point(QImage(":/IMG/bus-11.png"));

	_points[0x3001] = Point(QImage(":/IMG/police-11.png"));
	_points[0x3002] = Point(QImage(":/IMG/hospital-11.png"));
	_points[0x3003] = Point(QImage(":/IMG/town-hall-11.png"));
	_points[0x3006] = Point(QImage(":/IMG/entrance-alt1-11.png"));
	_points[0x3007] = Point(QImage(":/IMG/town-hall-11.png"));
	_points[0x3008] = Point(QImage(":/IMG/fire-station-11.png"));

	_points[0x4000] = Point(QImage(":/IMG/golf-11.png"));
	_points[0x4300] = Point(QImage(":/IMG/harbor-11.png"));
	_points[0x4400] = Point(QImage(":/IMG/fuel-11.png"));
	_points[0x4500] = Point(QImage(":/IMG/restaurant-11.png"));
	_points[0x4600] = Point(QImage(":/IMG/bar-11.png"));
	_points[0x4900] = Point(QImage(":/IMG/park-11.png"));
	_points[0x4a00] = Point(QImage(":/IMG/picnic-site-11.png"));
	_points[0x4c00] = Point(QImage(":/IMG/information-11.png"));
	_points[0x4800] = Point(QImage(":/IMG/campsite-11.png"));
	_points[0x4a00] = Point(QImage(":/IMG/picnic-site-11.png"));
	_points[0x4b00] = Point(QImage(":/IMG/hospital-11.png"));
	_points[0x4c00] = Point(QImage(":/IMG/information-11.png"));
	_points[0x4d00] = Point(QImage(":/IMG/parking-11.png"));
	_points[0x4d00].setTextFontSize(None);
	_points[0x4e00] = Point(QImage(":/IMG/toilet-11.png"));
	_points[0x4e00].setTextFontSize(None);
	_points[0x5000] = Point(QImage(":/IMG/drinking-water-11.png"));
	_points[0x5000].setTextFontSize(None);
	_points[0x5100] = Point(QImage(":/IMG/telephone-11.png"));
	_points[0x5200] = Point(QImage(":/IMG/viewpoint-11.png"));
	_points[0x5300] = Point(QImage(":/IMG/skiing-11.png"));
	_points[0x5400] = Point(QImage(":/IMG/swimming-11.png"));
	_points[0x5500] = Point(QImage(":/IMG/dam-11.png"));
	_points[0x5700] = Point(QImage(":/IMG/danger-11.png"));
	_points[0x5800] = Point(QImage(":/IMG/roadblock-11.png"));
	_points[0x5900] = Point(QImage(":/IMG/airport-11.png"));
	_points[0x5901] = Point(QImage(":/IMG/airport-11.png"));
	_points[0x5904] = Point(QImage(":/IMG/heliport-11.png"));

	_points[0x6401] = Point(QImage(":/IMG/bridge-11.png"));
	_points[0x6402] = Point(QImage(":/IMG/building-alt1-11.png"));
	_points[0x6403] = Point(QImage(":/IMG/cemetery-11.png"));
	_points[0x6404] = Point(QImage(":/IMG/religious-christian-11.png"));
	_points[0x6407] = Point(QImage(":/IMG/dam-11.png"));
	_points[0x6408] = Point(QImage(":/IMG/hospital-11.png"));
	_points[0x6409] = Point(QImage(":/IMG/dam-11.png"));
	_points[0x640d] = Point(QImage(":/IMG/communications-tower-11.png"));
	_points[0x640e] = Point(QImage(":/IMG/park-11.png"));
	_points[0x640f] = Point(QImage(":/IMG/post-11.png"));
	_points[0x6411] = Point(QImage(":/IMG/communications-tower-11.png"));

	_points[0x6508] = Point(QImage(":/IMG/waterfall-11.png"));
	_points[0x6513] = Point(QImage(":/IMG/wetland-11.png"));
	_points[0x6604] = Point(QImage(":/IMG/beach-11.png"));
	_points[0x6616] = Point(QImage(":/IMG/mountain-11.png"));


	// NT types
	_points[0x11401] = _points[TYPE(0x01)];
	_points[0x11402] = _points[TYPE(0x02)];
	_points[0x11403] = _points[TYPE(0x03)];
	_points[0x10b00] = _points[0x2a00];
	_points[0x10b01] = _points[0x2a01];
	_points[0x10b02] = _points[0x2a02];
	_points[0x10b03] = _points[0x2a03];
	_points[0x10b04] = _points[0x2a04];
	_points[0x10b05] = _points[0x2a05];
	_points[0x10b06] = _points[0x2a06];
	_points[0x10b07] = _points[0x2a07];
	_points[0x10b08] = _points[0x2a08];
	_points[0x10b09] = _points[0x2a09];
	_points[0x10b0a] = _points[0x2a0a];
	_points[0x10b0b] = _points[0x2a0b];
	_points[0x10b0c] = _points[0x2a0c];
	_points[0x10b0d] = _points[0x2a0d];
	_points[0x10b0e] = _points[0x2a0e];
	_points[0x10b0f] = _points[0x2a0f];
	_points[0x10b10] = _points[0x2a10];
	_points[0x10b11] = _points[0x2a11];
	_points[0x10c01] = _points[0x2b01];
	_points[0x10c02] = _points[0x2b02];
	_points[0x10c03] = _points[0x2b03];
	_points[0x10c04] = _points[0x2b04];
	_points[0x10d01] = _points[0x2c01];
	_points[0x10d02] = _points[0x2c02];
	_points[0x10d03] = _points[0x2c03];
	_points[0x10d04] = _points[0x2c04];
	_points[0x10d05] = _points[0x2c05];
	_points[0x10d06] = _points[0x2c06];
	_points[0x10d07] = _points[0x2c07];
	_points[0x10d08] = _points[0x2c08];
	_points[0x10d0a] = _points[0x2c0a];
	_points[0x10d0b] = _points[0x2c0b];
	_points[0x10d0d] = _points[0x2c0d];
	_points[0x10d0e] = _points[0x2c0e];
	_points[0x10d10] = _points[0x2c10];
	_points[0x10e01] = _points[0x2d01];
	_points[0x10e02] = _points[0x2d02];
	_points[0x10e03] = _points[0x2d03];
	_points[0x10e04] = _points[0x2d04];
	_points[0x10e05] = _points[0x2d05];
	_points[0x10e06] = _points[0x2d06];
	_points[0x10e07] = _points[0x2d07];
	_points[0x10e08] = _points[0x2d08];
	_points[0x10e09] = _points[0x2d09];
	_points[0x10e0a] = _points[0x2d0a];
	_points[0x10e0b] = _points[0x2d0b];
	_points[0x10f02] = _points[0x2e02];
	_points[0x10f03] = _points[0x2e03];
	_points[0x10f05] = _points[0x2e05];
	_points[0x10f06] = _points[0x2e06];
	_points[0x10f07] = _points[0x2e07];
	_points[0x10f08] = _points[0x2e08];
	_points[0x10f09] = _points[0x2e09];
	_points[0x10f0a] = _points[0x2e0a];
	_points[0x11001] = _points[0x2f01];
	_points[0x11002] = _points[0x2f02];
	_points[0x11003] = _points[0x2f03];
	_points[0x11004] = _points[0x2f04];
	_points[0x11005] = _points[0x2f05];
	_points[0x11006] = _points[0x2f06];
	_points[0x11007] = _points[0x2f07];
	_points[0x11008] = _points[0x2f08];
	_points[0x11009] = _points[0x2f09];
	_points[0x1100b] = _points[0x2f0b];
	_points[0x1100c] = _points[0x2f0c];
	_points[0x11010] = _points[0x2f10];
	_points[0x11012] = _points[0x2f12];
	_points[0x11013] = _points[0x2f13];
	_points[0x11017] = _points[0x2f17];
	_points[0x11101] = _points[0x3001];
	_points[0x11102] = _points[0x3002];
	_points[0x11103] = _points[0x3003];
	_points[0x11106] = _points[0x3006];
	_points[0x11107] = _points[0x3007];
	_points[0x11108] = _points[0x3008];

	// Marine stuff
	_points[0x10100] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10101] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10102] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10103] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10104] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10105] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10106] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10107] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10108] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10109] = Point(QImage(":/IMG/light-major.png"));
	_points[0x1010a] = Point(QImage(":/IMG/light-major.png"));
	_points[0x10200] = Point(QImage(":/IMG/buoy.png"));
	_points[0x10201] = Point(QImage(":/IMG/buoy.png"));
	_points[0x10202] = Point(QImage(":/IMG/buoy.png"));
	_points[0x10203] = Point(QImage(":/IMG/buoy.png"));
	_points[0x10204] = Point(QImage(":/IMG/buoy.png"));
	_points[0x10205] = Point(QImage(":/IMG/buoy.png"));
	_points[0x10206] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10207] = Point(QImage(":/IMG/spar-buoy.png"));
	_points[0x1020b] = Point(QImage(":/IMG/buoy.png"));
	_points[0x1020d] = Point(QImage(":/IMG/light-platform.png"));
	_points[0x1020e] = Point(QImage(":/IMG/beacon.png"));
	_points[0x1020f] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10210] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10211] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10212] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10213] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10214] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10215] = Point(QImage(":/IMG/beacon.png"));
	_points[0x10216] = Point(QImage(":/IMG/mooring-buoy.png"));
	_points[0x10306] = Point(QImage(":/IMG/church.png"));
	_points[0x1030a] = Point(QImage(":/IMG/triangulation-point.png"));
	_points[0x10400] = Point(QImage(":/IMG/obstruction.png"));
	_points[0x10401] = Point(QImage(":/IMG/obstruction.png"));
	_points[0x10402] = Point(QImage(":/IMG/wreck.png"));
	_points[0x10403] = Point(QImage(":/IMG/wreck-exposed.png"));
	_points[0x10408] = Point(QImage(":/IMG/obstruction-covers.png"));
	_points[0x1040a] = Point(QImage(":/IMG/rock-dangerous.png"));
	_points[0x1040c] = Point(QImage(":/IMG/rock-exposed.png"));
	_points[0x10701] = Point(QImage(":/IMG/anchorage.png"));
	_points[0x10703] = Point(QImage(":/IMG/yacht-harbor.png"));
	_points[0x10704] = Point(QImage(":/IMG/pile.png"));
	_points[0x10705] = Point(QImage(":/IMG/anchoring-prohibited.png"));
}

static bool readBitmap(SubFile *file, SubFile::Handle &hdl, QImage &img,
  int bpp)
{
	if (!bpp)
		return true;

	for (int y = 0; y < img.height(); y++) {
		for (int x = 0; x < img.width(); x += 8/bpp) {
			quint8 color;
			if (!file->readByte(hdl, &color))
				return false;

			for (int i = 0; i < 8/bpp && x + i < img.width(); i++) {
				int value = (i > 0) ? (color >>= bpp) : color;

				if (bpp == 4)
					value = value & 0xf;
				else if (bpp == 2)
					value = value & 0x3;
				else if (bpp == 1)
					value = value & 0x1;

				img.setPixel(x + i, y, value);
			}
		}
	}

	return true;
}

static bool readColor(SubFile *file, SubFile::Handle &hdl, QColor &color)
{
	quint8 b, g, r;

	if (!(file->readByte(hdl, &b) && file->readByte(hdl, &g)
	  && file->readByte(hdl, &r)))
		return false;

	color = qRgb(r, g, b);

	return true;
}

static bool skipLabel(SubFile *file, SubFile::Handle &hdl)
{
	quint32 len;

	if (!file->readVUInt32(hdl, len))
		return false;
	if (!file->seek(hdl, file->pos(hdl) + len))
		return false;

	return true;
}


bool Style::itemInfo(SubFile *file, SubFile::Handle &hdl,
  const Section &section, ItemInfo &info)
{
	quint16 t16_1, t16_2;
	quint8 t8;

	if (section.arrayItemSize == 5) {
		if (!(file->readUInt16(hdl, t16_1) && file->readUInt16(hdl, t16_2)
		  && file->readByte(hdl, &t8)))
			return false;
		info.offset = t16_2 | (t8<<16);
	} else if (section.arrayItemSize == 4) {
		if (!(file->readUInt16(hdl, t16_1) && file->readUInt16(hdl, t16_2)))
			return false;
		info.offset = t16_2;
	} else if (section.arrayItemSize == 3) {
		if (!(file->readUInt16(hdl, t16_1) && file->readByte(hdl, &t8)))
			return false;
		info.offset = t8;
	} else
		return false;

	t16_2 = (t16_1 >> 5) | (( t16_1 & 0x1f) << 11);
	info.type = t16_2 & 0x7F;
	info.subtype  = t16_1 & 0x1F;
	info.extended = t16_1 & 0x2000;

	return true;
}

bool Style::parsePolygons(SubFile *file, SubFile::Handle &hdl,
  const Section &section)
{
	if (!section.arrayItemSize)
		return section.arraySize ? false : true;

	for (quint32 i = 0; i < section.arraySize / section.arrayItemSize; i++) {
		if (!file->seek(hdl, section.arrayOffset + i * section.arrayItemSize))
			return false;
		ItemInfo info;
		if (!itemInfo(file, hdl, section, info))
			return false;
		quint32 type = info.extended
		  ? 0x10000 | (info.type << 8) | info.subtype : (info.type << 8);

		quint8 t8, flags;
		if (!(file->seek(hdl, section.offset + info.offset)
		  && file->readByte(hdl, &t8)))
			return false;
		flags = t8 & 0x0F;

		QColor c1, c2, c3, c4;
		QImage img(32, 32, QImage::Format_Indexed8);

		switch (flags) {
			case 0x01:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3) && readColor(file, hdl, c4)))
					return false;
				_polygons[type] = Polygon(QBrush(c1), QPen(c3, 2));
				break;

			case 0x06:
			case 0x07:
				if (!readColor(file, hdl, c1))
					return false;
				_polygons[type] = Polygon(QBrush(c1));
				break;

			case 0x08:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				img.setColorCount(2);
				img.setColor(0, c2.rgb());
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Polygon(QBrush(img));
				break;

			case 0x09:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3) && readColor(file, hdl, c4)))
					return false;

				img.setColorCount(2);
				img.setColor(0, c2.rgb());
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Polygon(QBrush(img));
				break;

			case 0x0B:
			case 0x0D:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3)))
					return false;

				img.setColorCount(2);
				img.setColor(0, (flags == 0x0B) ? qRgba(255, 255, 255, 0)
				  : c2.rgb());
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Polygon(QBrush(img));
				break;

			case 0x0E:
				if (!readColor(file, hdl, c1))
					return false;

				img.setColorCount(2);
				img.setColor(0, qRgba(255, 255, 255, 0));
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Polygon(QBrush(img));
				break;

			case 0x0F:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				img.setColorCount(2);
				img.setColor(0, qRgba(255, 255, 255, 0));
				img.setColor(1, c1.rgb());
				if (!readBitmap(file, hdl, img, 1))
					return false;

				_polygons[type] = Polygon(QBrush(img));
				break;

			default:
				return false;
		}
	}

	return true;
}

bool Style::parseLines(SubFile *file, SubFile::Handle &hdl,
  const Section &section)
{
	if (!section.arrayItemSize)
		return section.arraySize ? false : true;

	for (quint32 i = 0; i < section.arraySize / section.arrayItemSize; i++) {
		if (!file->seek(hdl, section.arrayOffset + i * section.arrayItemSize))
			return false;
		ItemInfo info;
		if (!itemInfo(file, hdl, section, info))
			return false;
		quint32 type = info.extended
		  ? 0x10000 | (info.type << 8) | info.subtype : (info.type << 8);

		quint8 t8_1, t8_2, flags, rows;
		if (!(file->seek(hdl, section.offset + info.offset)
		  && file->readByte(hdl, &t8_1) && file->readByte(hdl, &t8_2)))
			return false;
		flags = t8_1 & 0x07;
		rows = t8_1 >> 3;
		bool label = t8_2 & 0x01;
		bool fontInfo = t8_2 & 0x04;

		QColor c1, c2, c3, c4;
		quint8 w1, w2;

		switch (flags) {
			case 0x00:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, c2.rgb());
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Line(img);
				} else {
					if (!(file->readByte(hdl, &w1) && file->readByte(hdl, &w2)))
						return false;

					_lines[type] = (w2 > w1)
					  ? Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin), QPen(c2, w2, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin))
					  : Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin));
				}
				break;

			case 0x01:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3) && readColor(file, hdl, c4)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, c2.rgb());
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Line(img);
				} else {
					if (!(file->readByte(hdl, &w1) && file->readByte(hdl, &w2)))
						return false;

					_lines[type] = (w2 > w1)
					  ? Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin), QPen(c2, w2, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin))
					  : Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin));
				}
				break;

			case 0x03:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, qRgba(255, 255, 255, 0));
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Line(img);
				} else {
					if (!(file->readByte(hdl, &w1) && file->readByte(hdl, &w2)))
						return false;

					_lines[type] = Line(QPen(c1, w1, Qt::SolidLine,
					  Qt::RoundCap, Qt::RoundJoin));
				}
				break;

			case 0x05:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)
				  && readColor(file, hdl, c3)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, c2.rgb());
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Line(img);
				} else {
					if (!(file->readByte(hdl, &w1) && file->readByte(hdl, &w2)))
						return false;

					_lines[type] = (w2 > w1)
					  ? Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin), QPen(c2, w2, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin))
					  : Line(QPen(c1, w1, Qt::SolidLine, Qt::RoundCap,
						Qt::RoundJoin));
				}
				break;

			case 0x06:
				if (!readColor(file, hdl, c1))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, qRgba(255, 255, 255, 0));
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Line(img);
				} else {
					if (!file->readByte(hdl, &w1))
						return false;

					_lines[type] = Line(QPen(c1, w1, Qt::SolidLine,
					  Qt::RoundCap, Qt::RoundJoin));
				}
				break;

			case 0x07:
				if (!(readColor(file, hdl, c1) && readColor(file, hdl, c2)))
					return false;

				if (rows) {
					QImage img(32, rows, QImage::Format_Indexed8);
					img.setColorCount(2);
					img.setColor(0, qRgba(255,255,255,0));
					img.setColor(1, c1.rgb());
					if (!readBitmap(file, hdl, img, 1))
						return false;

					_lines[type] = Line(img);
				} else {
					if (!file->readByte(hdl, &w1))
						return false;

					_lines[type] = Line(QPen(c1, w1, Qt::SolidLine,
					  Qt::RoundCap, Qt::RoundJoin));
				}
				break;

			default:
				return false;
		}

		if (isContourLine(type)) {
			Line &l = _lines[type];
			l.setTextColor(l.foreground().color());
			l.setTextFontSize(Small);
		}

		if (label && !skipLabel(file, hdl))
			return false;

		if (fontInfo) {
			quint8 labelFlags;
			if (!file->readByte(hdl, &labelFlags))
				return false;
			if (labelFlags & 0x08) {
				if (!readColor(file, hdl, c1))
					return false;
				_lines[type].setTextColor(c1);
			}
			_lines[type].setTextFontSize((FontSize)(labelFlags & 0x07));
		}
	}

	return true;
}

static int colors2bpp(quint8 colors, quint8 flags)
{
	switch (flags) {
		case 0x00:
			if (colors < 3)
				return colors;
			else if (colors == 3)
				return 2;
			else if (colors < 16)
				return 4;
			else
				return 8;
		case 0x10:
			if (colors == 0)
				return 1;
			else if (colors < 3)
				return 2;
			else if (colors < 15)
				return 4;
			else
				return 8;
		case 0x20:
			if (colors == 0)
				return -1;
			else if (colors < 3)
				return colors;
			else if (colors < 4)
				return 2;
			else if (colors < 16)
				return 4;
			else
				return 8;
		default:
			return -1;
	}
}

static bool readColorTable(SubFile *file, SubFile::Handle &hdl, QImage& img,
  int colors, int bpp, bool transparent)
{
	img.setColorCount(colors);

	if (transparent) {
		quint8 byte;
		quint32 bits = 0, reg = 0, mask = 0x000000FF;

		for (int i = 0; i < colors; i++) {
			while (bits < 28) {
				if (!file->readByte(hdl, &byte))
					return false;

				mask = 0x000000FF << bits;
				reg  = reg & (~mask);
				reg  = reg | (byte << bits);
				bits += 8;
			}

			img.setColor(i, qRgba((reg >> 16) & 0x0FF, (reg >> 8) & 0x0FF,
			  reg & 0x0FF, ~((reg >> 24) & 0x0F) << 4));

			reg = reg >> 28;
			bits -= 28;
		}
		for (int i = colors; i < 1<<bpp; i++)
			img.setColor(i, qRgba(0, 0, 0, 0));
	} else {
		QColor color;

		for (int i = 0; i < colors; i++) {
			if (!readColor(file, hdl, color))
				return false;
			img.setColor(i, color.rgb());
		}
		for (int i = colors; i < 1<<bpp; i++)
			img.setColor(i, qRgba(0, 0, 0, 0));
	}

	return true;
}

bool Style::parsePoints(SubFile *file, SubFile::Handle &hdl,
  const Section &section)
{
	if (!section.arrayItemSize)
		return section.arraySize ? false : true;

	for (quint32 i = 0; i < section.arraySize / section.arrayItemSize; i++) {
		if (!file->seek(hdl, section.arrayOffset + i * section.arrayItemSize))
			return false;
		ItemInfo info;
		if (!itemInfo(file, hdl, section, info))
			return false;
		quint32 type = info.extended
		  ? 0x10000 | (info.type << 8) | info.subtype
		  : (info.type << 8) | info.subtype;

		quint8 t8_1, width, height, numColors, imgType;
		if (!(file->seek(hdl, section.offset + info.offset)
		  && file->readByte(hdl, &t8_1) && file->readByte(hdl, &width)
		  && file->readByte(hdl, &height) && file->readByte(hdl, &numColors)
		  && file->readByte(hdl, &imgType)))
			return false;

		bool label = t8_1 & 0x04;
		bool fontInfo = t8_1 & 0x08;

		int bpp = colors2bpp(numColors, imgType);
		if (bpp <= 0)
			continue;
		QImage img(width, height, QImage::Format_Indexed8);
		if (!readColorTable(file, hdl, img, numColors, bpp, imgType == 0x20))
			return false;
		if (!readBitmap(file, hdl, img, bpp))
			return false;
		_points[type] = Point(img);

		if (t8_1 == 0x03) {
			if (!(file->readByte(hdl, &numColors)
			  && file->readByte(hdl, &imgType)))
				return false;
			if ((bpp = colors2bpp(numColors, imgType)) < 0)
				continue;
			if (!readColorTable(file, hdl, img, numColors, bpp, imgType == 0x20))
				return false;
			if (!readBitmap(file, hdl, img, bpp))
				return false;
		} else if (t8_1 == 0x02) {
			if (!(file->readByte(hdl, &numColors)
			  && file->readByte(hdl, &imgType)))
				return false;
			if ((bpp = colors2bpp(numColors, imgType)) < 0)
				continue;
			if (!readColorTable(file, hdl, img, numColors, bpp, imgType == 0x20))
				return false;
		}

		if (label && !skipLabel(file, hdl))
			return false;

		if (fontInfo) {
			quint8 labelFlags;
			QColor color;
			if (!file->readByte(hdl, &labelFlags))
				return false;
			if (labelFlags & 0x08) {
				if (!readColor(file, hdl, color))
					return false;
				_points[type].setTextColor(color);
			}
			_points[type].setTextFontSize((FontSize)(labelFlags & 0x07));
		}
	}

	return true;
}

bool Style::parseDrawOrder(SubFile *file, SubFile::Handle &hdl,
  const Section &order)
{
	QList<quint32> drawOrder;

	if (!order.arrayItemSize)
		return order.arraySize ? false : true;

	if (!file->seek(hdl, order.arrayOffset))
		return false;

	for (quint32 i = 0; i < order.arraySize / order.arrayItemSize; i++) {
		quint8 type;
		quint32 subtype;

		if (!(file->readByte(hdl, &type) && file->readUInt32(hdl, subtype)))
			return false;

		if (!subtype)
			drawOrder.append(((quint32)type) << 8);
		else {
			for (int j = 0; j < 32; j++) {
				quint32 mask = 1 << j;
				if (subtype & mask)
					drawOrder.append(0x010000 | (((quint32)type) << 8) | j);
			}
		}
	}

	_drawOrder = drawOrder;

	return true;
}

bool Style::parseTYPFile(SubFile *file)
{
	SubFile::Handle hdl(file);
	Section points, lines, polygons, order;
	quint16 tmp16, codepage;

	if (!(file->seek(hdl, 0x15) && file->readUInt16(hdl, codepage)
	  && file->readUInt32(hdl, points.offset)
	  && file->readUInt32(hdl, points.size)
	  && file->readUInt32(hdl, lines.offset)
	  && file->readUInt32(hdl, lines.size)
	  && file->readUInt32(hdl, polygons.offset)
	  && file->readUInt32(hdl, polygons.size)))
		return false;

	if (!(file->readUInt16(hdl, tmp16) && file->readUInt16(hdl, tmp16)))
		return false;

	if (!(file->readUInt32(hdl, points.arrayOffset)
	  && file->readUInt16(hdl, points.arrayItemSize)
	  && file->readUInt32(hdl, points.arraySize)
	  && file->readUInt32(hdl, lines.arrayOffset)
	  && file->readUInt16(hdl, lines.arrayItemSize)
	  && file->readUInt32(hdl, lines.arraySize)
	  && file->readUInt32(hdl, polygons.arrayOffset)
	  && file->readUInt16(hdl, polygons.arrayItemSize)
	  && file->readUInt32(hdl, polygons.arraySize)
	  && file->readUInt32(hdl, order.arrayOffset)
	  && file->readUInt16(hdl, order.arrayItemSize)
	  && file->readUInt32(hdl, order.arraySize)))
		return false;

	if (!(parsePoints(file, hdl, points) && parseLines(file, hdl, lines)
	  && parsePolygons(file, hdl, polygons)
	  && parseDrawOrder(file, hdl, order))) {
		qWarning("%s: Invalid TYP file, using default style",
		  qPrintable(file->fileName()));
		return false;
	}

	return true;
}

Style::Style(SubFile *typ)
{
	defaultLineStyle();
	defaultPolygonStyle();
	defaultPointStyle();

	if (typ)
		parseTYPFile(typ);
}

const Style::Line &Style::line(quint32 type) const
{
	static Line null;

	QMap<quint32, Line>::const_iterator it = _lines.find(type);
	return (it == _lines.constEnd()) ? null : *it;
}

const Style::Polygon &Style::polygon(quint32 type) const
{
	static Polygon null;

	QMap<quint32, Polygon>::const_iterator it = _polygons.find(type);
	return (it == _polygons.constEnd()) ? null : *it;
}

const Style::Point &Style::point(quint32 type) const
{
	static Point null;

	QMap<quint32, Point>::const_iterator it = _points.find(type);
	return (it == _points.constEnd()) ? null : *it;
}

#ifndef QT_NO_DEBUG
static QString penColor(const QPen &pen)
{
	return (pen == Qt::NoPen) ? "None" : pen.color().name();
}

static QString brushColor(const QBrush &brush)
{
	return (brush == Qt::NoBrush) ? "None" : brush.color().name();
}

QDebug operator<<(QDebug dbg, const Style::Polygon &polygon)
{
	dbg.nospace() << "Polygon(" << brushColor(polygon.brush()) << ", "
	  << penColor(polygon.pen()) << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Style::Line &line)
{
	dbg.nospace() << "Line(" << penColor(line.foreground()) << ", "
	  << penColor(line.background()) << ", " << !line.img().isNull() << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const Style::Point &point)
{
	dbg.nospace() << "Point(" << point.textFontSize() << ", "
	  << point.textColor() << ", " << !point.img().isNull() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
