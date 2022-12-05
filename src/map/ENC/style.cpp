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

void Style::polygonStyle()
{
	_polygons[TYPE(M_COVR)] = Polygon(QBrush("#ffffff"));
	_polygons[TYPE(LNDARE)] = Polygon(QBrush("#e8e064"));
	_polygons[TYPE(BUAARE)] = Polygon(QBrush("#d98b21"));
	_polygons[TYPE(BUISGL)] = Polygon(QBrush("#d98b21"),
	  QPen(QColor("#966118"), 1.5));
	_polygons[TYPE(BRIDGE)] = Polygon(QBrush("#a58140"));
	_polygons[TYPE(I_BRIDGE)] = Polygon(QBrush("#a58140"));
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
	_polygons[TYPE(UWTROC)] = Polygon(Qt::NoBrush, QPen(QColor("#000000"), 1.5,
	  Qt::DotLine));
	_polygons[TYPE(PONTON)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(I_PONTON)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(HULKES)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(I_HULKES)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(DRYDOC)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(SLCONS)] = Polygon(Qt::NoBrush, QPen(QColor("#333333"), 1.5,
	  Qt::DashLine));
	_polygons[TYPE(I_SLCONS)] = Polygon(Qt::NoBrush, QPen(QColor("#333333"), 1.5,
	  Qt::DashLine));
	_polygons[TYPE(LAKARE)] = Polygon(QBrush("#9fc4e1"),
	  QPen(QColor("#000000"), 1));
	_polygons[TYPE(CANALS)] = Polygon(QBrush("#9fc4e1"),
	  QPen(QColor("#000000"), 1));
	_polygons[TYPE(RIVERS)] = Polygon(QBrush("#9fc4e1"));
	_polygons[TYPE(DYKCON)] = Polygon(QBrush(QColor("#9fc4e1"),
	  Qt::Dense4Pattern), QPen(QColor("#000000"), 1));
	_polygons[TYPE(AIRARE)] = Polygon(QBrush("#333333"));
	_polygons[TYPE(TSEZNE)] = Polygon(QBrush("#80fcb4fc"));
	_polygons[TYPE(DRGARE)] = Polygon(QBrush(QColor("#a0a0ff"),
	  Qt::Dense4Pattern));
	_polygons[TYPE(UNSARE)] = Polygon(QBrush("#999999"));
	_polygons[SUBTYPE(RESARE, 9)] = Polygon(QBrush(QColor("#ff0000"),
	  Qt::BDiagPattern));
	_polygons[SUBTYPE(RESARE, 2)] = Polygon(QImage(":/marine/noanchor-line.png"));
	_polygons[SUBTYPE(I_RESARE, 2)] = Polygon(QImage(":/marine/noanchor-line.png"));
	_polygons[SUBTYPE(RESARE, 17)] = Polygon(
	  QImage(":/marine/entry-prohibited-line.png"));
	_polygons[SUBTYPE(I_RESARE, 17)] = Polygon(
	  QImage(":/marine/entry-prohibited-line.png"));
	_polygons[SUBTYPE(ACHARE, 1)] = Polygon(QImage(":/marine/anchor-line.png"));
	_polygons[SUBTYPE(I_ACHARE, 1)] = Polygon(QImage(":/marine/anchor-line.png"));
	_polygons[TYPE(PRCARE)] = Polygon(QBrush(QColor("#eb49eb"),
	  Qt::BDiagPattern));
	_polygons[TYPE(DAMCON)] = Polygon(QBrush("#d98b21"), QPen(QColor("#000000"),
	  1));
	_polygons[TYPE(DRYDOC)] = Polygon(QBrush("#ebab54"), QPen(QColor("#000000"),
	  1));
	_polygons[TYPE(PYLONS)] = Polygon(QBrush("#a58140"), QPen(QColor("#000000"),
	  1));
	_polygons[TYPE(FLODOC)] = Polygon(QBrush("#333333"), QPen(QColor("#000000"),
	  1));
	_polygons[TYPE(I_FLODOC)] = Polygon(QBrush("#333333"),
	  QPen(QColor("#000000"), 1));
	_polygons[TYPE(DWRTPT)] = Polygon(QImage(":/marine/dw-route-line.png"));
	_polygons[TYPE(MORFAC)] = Polygon(QBrush("#e8e064"), QPen(QColor("#000000"),
	  2));
	_polygons[TYPE(GATCON)] = Polygon(QBrush("#000000"));
	_polygons[TYPE(I_GATCON)] = Polygon(QBrush("#000000"));
	_polygons[TYPE(I_TERMNL)] = Polygon(QBrush(QColor("#b8b04b")),
	  QPen(QColor("#966118")));
	_polygons[TYPE(SILTNK)] = Polygon(QBrush("#d98b21"), QPen(QColor("#966118"),
	  2));
	_polygons[TYPE(LOKBSN)] = Polygon(QBrush(QColor("#333333"),
	  Qt::Dense7Pattern));
	_polygons[TYPE(I_LOKBSN)] = Polygon(QBrush(QColor("#333333"),
	  Qt::Dense7Pattern));
	_polygons[TYPE(TUNNEL)] = Polygon(Qt::NoBrush, QPen(QColor("#a58140"), 1.5,
	  Qt::DashLine));
	_polygons[TYPE(CBLARE)] = Polygon(QImage(":/marine/cable-area-line.png"));
	_polygons[TYPE(PIPARE)] = Polygon(QImage(":/marine/pipeline-area-line.png"));

	_drawOrder
	  << TYPE(M_COVR) << TYPE(LNDARE) << SUBTYPE(DEPARE, 0)
	  << SUBTYPE(DEPARE, 1) << SUBTYPE(DEPARE, 2) << SUBTYPE(DEPARE, 3)
	  << TYPE(UNSARE) << SUBTYPE(DEPARE, 4) << SUBTYPE(DEPARE, 5)
	  << SUBTYPE(DEPARE, 6) << TYPE(LAKARE) << TYPE(CANALS) << TYPE(DYKCON)
	  << TYPE(RIVERS) << TYPE(DRGARE) << TYPE(FAIRWY) << TYPE(LOKBSN)
	  << TYPE(I_LOKBSN) << TYPE(BUAARE) << TYPE(BUISGL) << TYPE(SILTNK)
	  << TYPE(AIRARE) << TYPE(BRIDGE) << TYPE(I_BRIDGE) << TYPE(TUNNEL)
	  << TYPE(I_TERMNL) << TYPE(SLCONS) << TYPE(I_SLCONS) << TYPE(PONTON)
	  << TYPE(I_PONTON) << TYPE(HULKES) << TYPE(I_HULKES) << TYPE(FLODOC)
	  << TYPE(I_FLODOC) << TYPE(DRYDOC) << TYPE(DAMCON) << TYPE(PYLONS)
	  << TYPE(MORFAC) << TYPE(GATCON) << TYPE(I_GATCON) << TYPE(DMPGRD)
	  << TYPE(TSEZNE) << TYPE(OBSTRN) << TYPE(UWTROC) << TYPE(DWRTPT)
	  << SUBTYPE(ACHARE, 1) << SUBTYPE(I_ACHARE, 1) << SUBTYPE(RESARE, 9)
	  << SUBTYPE(RESARE, 2) << SUBTYPE(I_RESARE, 2) << SUBTYPE(RESARE, 17)
	  << SUBTYPE(I_RESARE, 17) << TYPE(CBLARE) << TYPE(PIPARE) << TYPE(PRCARE);
}

void Style::lineStyle()
{
	_lines[TYPE(BUISGL)] = Line(QPen(QColor("#966118"), 1.5));
	_lines[TYPE(DEPCNT)] = Line(QPen(QColor("#659aef"), 1, Qt::SolidLine));
	_lines[TYPE(DEPCNT)].setTextColor(QColor("#558adf"));
	_lines[TYPE(DEPCNT)].setTextFontSize(Small);
	_lines[TYPE(CBLOHD)] = Line(QImage(":/marine/cable-line.png"));
	_lines[TYPE(I_CBLOHD)] = Line(QImage(":/marine/cable-line.png"));
	_lines[TYPE(BRIDGE)] = Line(QPen(QColor("#a58140"), 3, Qt::SolidLine));
	_lines[TYPE(I_BRIDGE)] = Line(QPen(QColor("#a58140"), 3, Qt::SolidLine));
	_lines[TYPE(CBLSUB)] = Line(QImage(":/marine/cable.png"));
	_lines[TYPE(CBLSUB)].setTextFontSize(Small);
	_lines[TYPE(PIPSOL)] = Line(QImage(":/marine/pipeline.png"));
	_lines[TYPE(PIPSOL)].setTextFontSize(Small);
	_lines[TYPE(NAVLNE)] = Line(QPen(QColor("#eb49eb"), 1, Qt::DashLine));
	_lines[TYPE(COALNE)] = Line(QPen(QColor("#000000"), 1, Qt::SolidLine));
	_lines[TYPE(SLCONS)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
	_lines[TYPE(I_SLCONS)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
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
	_lines[TYPE(I_FERYRT)] = Line(QImage(":/marine/ferry-line.png"));
	_lines[TYPE(RAILWY)] = Line(railroad());
	_lines[TYPE(ROADWY)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
	_lines[TYPE(GATCON)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
	_lines[TYPE(I_GATCON)] = Line(QPen(QColor("#000000"), 2, Qt::SolidLine));
	_lines[TYPE(TSELNE)] = Line(QPen(QColor("#80fcb4fc"), 4, Qt::SolidLine));
	_lines[TYPE(I_WTWAXS)] = Line(QPen(QColor("#000000"), 0, Qt::DashLine));
	_lines[SUBTYPE(RECTRC, 1)] = Line(QPen(QColor("#000000"), 0, Qt::SolidLine));
	_lines[SUBTYPE(RECTRC, 2)] = Line(QPen(QColor("#000000"), 0, Qt::DashLine));
	_lines[SUBTYPE(RCRTCL, 1)] = Line(QPen(QColor("#eb49eb"), 0, Qt::SolidLine));
	_lines[SUBTYPE(RCRTCL, 2)] = Line(QPen(QColor("#eb49eb"), 0, Qt::DashLine));
	_lines[TYPE(FAIRWY)] = Line(QPen(QColor("#888888"), 1, Qt::DashDotDotLine));
	_lines[TYPE(BERTHS)] = Line(QPen(QColor("#333333"), 2));
	_lines[TYPE(I_BERTHS)] = Line(QPen(QColor("#333333"), 2));
	_lines[TYPE(FNCLNE)] = Line(QImage(":/marine/fence-line.png"));
	_lines[TYPE(CONVYR)] = Line(QImage(":/marine/conveyor-line.png"));
	_lines[TYPE(PIPOHD)] = Line(QImage(":/marine/pipeline-overhead.png"));
	_lines[TYPE(I_PIPOHD)] = Line(QImage(":/marine/pipeline-overhead.png"));
	_lines[TYPE(CANALS)] = Line(QPen(QColor("#9fc4e1"), 2));
}

void Style::pointStyle()
{
	_points[SUBTYPE(BUAARE, 1)].setTextFontSize(Large);
	_points[SUBTYPE(BUAARE, 5)].setTextFontSize(Large);
	_points[SUBTYPE(BUAARE, 3)].setTextFontSize(Small);
	_points[SUBTYPE(BUAARE, 6)].setTextFontSize(Small);
	_points[SUBTYPE(BUAARE, 0)].setTextFontSize(Small);
	_points[TYPE(SOUNDG)].setTextFontSize(Small);
	_points[TYPE(LIGHTS)] = Point(QImage(":/marine/light-major.png"), Small);
	_points[TYPE(BOYCAR)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYINB)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYISD)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYLAT)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(I_BOYLAT)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYSAW)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BOYSPP)] = Point(QImage(":/marine/buoy.png"), Small);
	_points[TYPE(BCNISD)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(BCNLAT)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(I_BCNLAT)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(BCNSAW)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[TYPE(BCNSPP)] = Point(QImage(":/marine/beacon.png"), Small);
	_points[SUBTYPE(LNDMRK, 3)] = Point(QImage(":/marine/chimney.png"));
	_points[SUBTYPE(LNDMRK, 9)] = Point(QImage(":/marine/monument.png"));
	_points[SUBTYPE(LNDMRK, 20)] = Point(QImage(":/marine/church.png"));
	_points[SUBTYPE(LNDMRK, 17)] = Point(QImage(":/marine/tower.png"));
	_points[SUBTYPE(LNDMRK, 19)] = Point(QImage(":/marine/windmotor.png"));
	_points[TYPE(LNDELV)] = Point(QImage(":/marine/triangulation-point.png"));
	_points[TYPE(OBSTRN)] = Point(QImage(":/marine/obstruction.png"), Small);
	_points[SUBTYPE(WRECKS, 1)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 2)] = Point(QImage(":/marine/wreck-dangerous.png"),
	  Small);
	_points[SUBTYPE(WRECKS, 3)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 4)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 5)] = Point(QImage(":/marine/wreck-exposed.png"));
	_points[SUBTYPE(UWTROC, 1)] = Point(QImage(":/marine/rock-exposed.png"),
	  Small);
	_points[SUBTYPE(UWTROC, 2)] = Point(QImage(":/marine/rock-exposed.png"),
	  Small);
	_points[SUBTYPE(UWTROC, 3)] = Point(QImage(":/marine/rock-dangerous.png"),
	  Small);
	_points[SUBTYPE(UWTROC, 4)] = Point(QImage(":/marine/rock-dangerous.png"),
	  Small);
	_points[SUBTYPE(UWTROC, 5)] = Point(QImage(":/marine/rock-dangerous.png"),
	  Small);
	_points[SUBTYPE(HRBFAC, 4)] = Point(QImage(":/marine/fishing-harbor.png"));
	_points[SUBTYPE(HRBFAC, 5)] = Point(QImage(":/marine/yacht-harbor.png"));
	_points[TYPE(ACHBRT)] = Point(QImage(":/marine/anchorage.png"));
	_points[TYPE(I_ACHBRT)] = Point(QImage(":/marine/anchorage.png"));
	_points[TYPE(OFSPLF)] = Point(QImage(":/marine/platform.png"));
	_points[TYPE(PILPNT)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 1)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 3)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 5)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 7)] = Point(QImage(":/marine/mooring-buoy.png"),
	  Small);
	_points[TYPE(CRANES)] = Point(QImage(":/marine/crane.png"));
	_points[TYPE(I_CRANES)] = Point(QImage(":/marine/crane.png"));
	_points[SUBTYPE(I_DISMAR, 1)] = Point(QImage(":/marine/distance-mark.png"));
	_points[SUBTYPE(I_DISMAR, 1)].setTextColor(QColor("#ffffff"));
	_points[SUBTYPE(I_DISMAR, 1)].setTextFontSize(Small);
	_points[SUBTYPE(I_DISMAR, 2)] = Point(QImage(":/marine/distance-mark-land.png"));
	_points[SUBTYPE(I_DISMAR, 2)].setTextFontSize(Small);
	_points[SUBTYPE(I_DISMAR, 3)] = Point(QImage(":/marine/distance-mark-land.png"));
	_points[SUBTYPE(I_DISMAR, 3)].setTextFontSize(Small);
	_points[TYPE(CGUSTA)] = Point(QImage(":/marine/coast-guard.png"));
	_points[TYPE(RDOSTA)] = Point(QImage(":/marine/radio.png"));
	_points[TYPE(RADSTA)] = Point(QImage(":/marine/radar.png"));
	_points[TYPE(RTPBCN)] = Point(QImage(":/marine/radar-transponder.png"));
	_points[TYPE(SILTNK)] = Point(QImage(":/marine/silo.png"));
	_points[TYPE(I_TRNBSN)] = Point(QImage(":/marine/turning-basin.png"));

	_points[SUBTYPE(SMCFAC, 7)] = Point(QImage(":/POI/restaurant-11.png"));
	_points[SUBTYPE(SMCFAC, 11)] = Point(QImage(":/POI/pharmacy-11.png"));
	_points[SUBTYPE(SMCFAC, 12)] = Point(QImage(":/POI/drinking-water-11.png"));
	_points[SUBTYPE(SMCFAC, 13)] = Point(QImage(":/POI/fuel-11.png"));
	_points[SUBTYPE(SMCFAC, 18)] = Point(QImage(":/POI/toilet-11.png"));
	_points[SUBTYPE(SMCFAC, 20)] = Point(QImage(":/POI/telephone-11.png"));
	_points[SUBTYPE(SMCFAC, 22)] = Point(QImage(":/POI/parking-11.png"));
	_points[SUBTYPE(SMCFAC, 25)] = Point(QImage(":/POI/campsite-11.png"));
	_points[SUBTYPE(BUISGL, 2)] = Point(QImage(":/POI/harbor-11.png"));
	_points[SUBTYPE(BUISGL, 5)] = Point(QImage(":/POI/hospital-11.png"));
	_points[SUBTYPE(BUISGL, 6)] = Point(QImage(":/POI/post-11.png"));
	_points[SUBTYPE(BUISGL, 7)] = Point(QImage(":/POI/lodging-11.png"));
	_points[SUBTYPE(BUISGL, 9)] = Point(QImage(":/POI/police-11.png"));
	_points[SUBTYPE(BUISGL, 13)] = Point(QImage(":/POI/bank-11.png"));
	_points[SUBTYPE(BUISGL, 19)] = Point(QImage(":/POI/school-11.png"));
	_points[SUBTYPE(BUISGL, 20)] = Point(QImage(":/POI/religious-christian-11.png"));
	_points[SUBTYPE(BUISGL, 22)] = Point(QImage(":/POI/religious-jewish-11.png"));
	_points[SUBTYPE(BUISGL, 26)] = Point(QImage(":/POI/religious-muslim-11.png"));
	_points[SUBTYPE(BUISGL, 42)] = Point(QImage(":/POI/bus-11.png"));
}

Style::Style()
{
	polygonStyle();
	lineStyle();
	pointStyle();
}

const Style::Line &Style::line(uint type) const
{
	static Line null;

	QMap<uint, Line>::const_iterator it = _lines.find(type);
	return (it == _lines.constEnd()) ? null : *it;
}

const Style::Polygon &Style::polygon(uint type) const
{
	static Polygon null;

	QMap<uint, Polygon>::const_iterator it = _polygons.find(type);
	return (it == _polygons.constEnd()) ? null : *it;
}

const Style::Point &Style::point(uint type) const
{
	static Point null;

	QMap<uint, Point>::const_iterator it = _points.find(type);
	return (it == _points.constEnd()) ? null : *it;
}
