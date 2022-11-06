#include <QPainter>
#include "style.h"

using namespace ENC;

static QImage railroad()
{
	QImage img(16, 4, QImage::Format_ARGB32_Premultiplied);
	img.fill(Qt::black);
	QPainter p(&img);
	p.setPen(QPen(Qt::white, 2));
	p.drawLine(9, 2, 15, 2);

	return img;
}

void Style::defaultPolygonStyle()
{
	_polygons[TYPE(M_COVR)] = Polygon(QBrush("#ffffff"));
	_polygons[TYPE(LNDARE)] = Polygon(QBrush("#e8e064"));
	_polygons[TYPE(BUAARE)] = Polygon(QBrush("#d98b21"));
	_polygons[TYPE(BUISGL)] = Polygon(QBrush("#d98b21"),
	  QPen(QColor("#966118"), 1.5));
	_polygons[TYPE(BRIDGE)] = Polygon(QBrush("#a58140"));
	_polygons[SUBTYPE(DEPARE, 0)] = Polygon(QBrush("#98c064"));
	_polygons[SUBTYPE(DEPARE, 1)] = Polygon(QBrush("#a0a0ff"));
	_polygons[SUBTYPE(DEPARE, 2)] = Polygon(QBrush("#b0b0ff"));
	_polygons[SUBTYPE(DEPARE, 3)] = Polygon(QBrush("#c0c0ff"));
	_polygons[SUBTYPE(DEPARE, 4)] = Polygon(QBrush("#c0d0ff"));
	_polygons[SUBTYPE(DEPARE, 5)] = Polygon(QBrush("#c0e0ff"));
	_polygons[SUBTYPE(DEPARE, 6)] = Polygon(QBrush("#ffffff"));
	_polygons[TYPE(DMPGRD)] = Polygon(QBrush(QColor("#a3a3a3"),
	  Qt::Dense3Pattern));
	_polygons[TYPE(FAIRWY)] = Polygon(Qt::NoBrush, QPen(QColor("#888888"), 1,
	  Qt::DashDotDotLine));
	_polygons[TYPE(OBSTRN)] = Polygon(Qt::NoBrush, QPen(QColor("#000000"), 1.5,
	  Qt::DotLine));
	_polygons[TYPE(PONTON)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(SLCONS)] = Polygon(Qt::NoBrush, QPen(QColor("#333333"), 1.5,
	  Qt::DashLine));
	_polygons[TYPE(ACHARE)] = Polygon(Qt::NoBrush, QPen(QColor("#e728e7"), 1,
	  Qt::DashDotLine));
	_polygons[TYPE(LAKARE)] = Polygon(QBrush("#9fc4e1"),
	  QPen(QColor("#000000"), 1));
	_polygons[TYPE(CANALS)] = Polygon(QBrush("#9fc4e1"),
	  QPen(QColor("#000000"), 1));
	_polygons[TYPE(RIVERS)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(DYKCON)] = Polygon(QBrush(QColor("#9fc4e1"),
	  Qt::Dense4Pattern), QPen(QColor("#000000"), 1));
	_polygons[TYPE(AIRARE)] = Polygon(QBrush("#333333"));
	_polygons[SUBTYPE(RESARE, 9)] = Polygon(QBrush(QColor("#ff0000"),
	  Qt::BDiagPattern), QPen(QColor("#ff0000"), 1));
	_polygons[TYPE(TSEZNE)] = Polygon(QBrush("#80fcb4fc"));
	_polygons[TYPE(DRGARE)] = Polygon(QBrush(QColor("#a0a0ff"),
	  Qt::Dense4Pattern));
	_polygons[TYPE(UNSARE)] = Polygon(QBrush("#999999"));

	_drawOrder
	  << TYPE(M_COVR) << TYPE(LNDARE) << SUBTYPE(DEPARE, 0)
	  << SUBTYPE(DEPARE, 1) << SUBTYPE(DEPARE, 2) << SUBTYPE(DEPARE, 3)
	  << TYPE(UNSARE) << SUBTYPE(DEPARE, 4) << SUBTYPE(DEPARE, 5)
	  << SUBTYPE(DEPARE, 6) << TYPE(LAKARE) << TYPE(CANALS) << TYPE(DYKCON)
	  << TYPE(RIVERS) << TYPE(DRGARE) << TYPE(FAIRWY) << TYPE(BUAARE)
	  << TYPE(BUISGL) << TYPE(AIRARE) << TYPE(BRIDGE) << TYPE(SLCONS)
	  << TYPE(PONTON) << TYPE(DMPGRD) << TYPE(TSEZNE) << TYPE(OBSTRN)
	  << TYPE(ACHARE) << SUBTYPE(RESARE, 9) << TYPE(154);
}

void Style::defaultLineStyle()
{
	_lines[TYPE(DEPCNT)] = Line(QPen(QColor("#659aef"), 1, Qt::SolidLine));
	_lines[TYPE(DEPCNT)].setTextColor(QColor("#558adf"));
	_lines[TYPE(DEPCNT)].setTextFontSize(Small);
	_lines[TYPE(CBLOHD)] = Line(QImage(":/marine/cable-line.png"));
	_lines[TYPE(BRIDGE)] = Line(QPen(QColor("#a58140"), 3, Qt::SolidLine));
	_lines[TYPE(CBLSUB)] = Line(QImage(":/marine/cable.png"));
	_lines[TYPE(CBLSUB)].setTextFontSize(Small);
	_lines[TYPE(PIPSOL)] = Line(QImage(":/marine/pipeline.png"));
	_lines[TYPE(NAVLNE)] = Line(QPen(QColor("#eb49eb"), 1, Qt::DashLine));
	_lines[TYPE(COALNE)] = Line(QPen(QColor("#000000"), 1, Qt::SolidLine));
	_lines[TYPE(SLCONS)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
	_lines[TYPE(PONTON)] = Line(QPen(QColor("#333333"), 1, Qt::SolidLine));
	_lines[TYPE(DYKCON)] = Line(QPen(QColor("#333333"), 2, Qt::SolidLine));
	_lines[TYPE(RIVERS)] = Line(QPen(QColor("#000000"), 1, Qt::SolidLine));
	_lines[TYPE(TSSBND)] = Line(QPen(QColor("#eb49eb"), 2, Qt::DashLine));
	_lines[TYPE(LNDELV)] = Line(QPen(QColor("#999440"), 1, Qt::SolidLine));
	_lines[TYPE(LNDELV)].setTextColor(QColor("#797420"));
	_lines[TYPE(LNDELV)].setTextFontSize(Small);
	_lines[TYPE(SLOTOP)] = Line(QPen(QColor("#797420"), 1, Qt::SolidLine));
	_lines[TYPE(OBSTRN)] = Line(QPen(QColor("#000000"), 1.5, Qt::DotLine));
	_lines[TYPE(FERYRT)] = Line(QImage(":/marine/ferry-line.png"));
	_lines[TYPE(RAILWY)] = Line(railroad());
	_lines[TYPE(ROADWY)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
	_lines[TYPE(GATCON)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
}

void Style::defaultPointStyle()
{
	_points[TYPE(BUAARE)].setTextFontSize(Large);
	_points[TYPE(SOUNDG)].setTextFontSize(Small);
	_points[TYPE(LIGHTS)] = Point(QImage(":/marine/light-major.png"), Small);
	_points[TYPE(BOYCAR)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYINB)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYISD)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYLAT)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYSAW)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYSPP)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BCNISD)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(BCNLAT)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(BCNSAW)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(BCNSPP)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[SUBTYPE(LNDMRK, 3)] = Point(QImage(":/marine/chimney.png"));
	_points[SUBTYPE(LNDMRK, 20)] = Point(QImage(":/marine/church.png"));
	_points[SUBTYPE(LNDMRK, 17)] = Point(QImage(":/marine/tower.png"));
	_points[TYPE(LNDELV)] = Point(QImage(":/marine/triangulation-point.png"));
	_points[TYPE(OBSTRN)] = Point(QImage(":/marine/obstruction.png"), Small);
	_points[TYPE(WRECKS)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 1)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 2)] = Point(QImage(":/marine/wreck-dangerous.png"),
	  Small);
	_points[SUBTYPE(WRECKS, 3)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 4)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 5)] = Point(QImage(":/marine/wreck-exposed.png"));
	_points[TYPE(UWTROC)] = Point(QImage(":/marine/rock-dangerous.png"), Small);
	_points[SUBTYPE(HRBFAC, 5)] = Point(QImage(":/marine/yacht-harbor.png"));
	_points[TYPE(ACHBRT)] = Point(QImage(":/marine/anchorage.png"));
	_points[TYPE(OFSPLF)] = Point(QImage(":/marine/platform.png"));
	_points[TYPE(PILPNT)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 1)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 5)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 7)] = Point(QImage(":/marine/mooring-buoy.png"),
	  Small);
}

Style::Style()
{
	defaultPolygonStyle();
	defaultLineStyle();
	defaultPointStyle();
}

const Style::Line &Style::line(quint32 type) const
{
	static Line null;

	QMap<uint, Line>::const_iterator it = _lines.find(type);
	return (it == _lines.constEnd()) ? null : *it;
}

const Style::Polygon &Style::polygon(quint32 type) const
{
	static Polygon null;

	QMap<uint, Polygon>::const_iterator it = _polygons.find(type);
	return (it == _polygons.constEnd()) ? null : *it;
}

const Style::Point &Style::point(quint32 type) const
{
	static Point null;

	QMap<uint, Point>::const_iterator it = _points.find(type);
	return (it == _points.constEnd()) ? null : *it;
}
