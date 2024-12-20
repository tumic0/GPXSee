#include <QPainter>
#include "common/util.h"
#include "objects.h"
#include "style.h"

using namespace ENC;
using namespace Util;

static QImage railroad(qreal ratio)
{
	QImage img(16 * ratio, 4 * ratio, QImage::Format_ARGB32_Premultiplied);
	img.setDevicePixelRatio(ratio);
	img.fill(Qt::black);
	QPainter p(&img);
	p.setPen(QPen(Qt::white, 2));
	p.drawLine(9, 2, 15, 2);

	return img;
}

static QFont pixelSizeFont(int pixelSize)
{
	QFont f;
	f.setPixelSize(pixelSize);
	return f;
}

void Style::polygonStyle()
{
	_polygons[TYPE(M_COVR)] = Polygon(QBrush(QColor(0xff, 0xff, 0xff)));
	_polygons[TYPE(LNDARE)] = Polygon(QBrush(QColor(0xe8, 0xe0, 0x64)));
	_polygons[TYPE(BUAARE)] = Polygon(QBrush(QColor(0xd9, 0x8b, 0x21)));
	_polygons[TYPE(BUISGL)] = Polygon(QBrush(QColor(0xd9, 0x8b, 0x21)),
	  QPen(QColor(0x96, 0x61, 0x18), 1.5));
	_polygons[TYPE(BRIDGE)] = Polygon(QBrush(QColor(0xa5, 0x81, 0x40)));
	_polygons[TYPE(I_BRIDGE)] = Polygon(QBrush(QColor(0xa5, 0x81, 0x40)));
	_polygons[SUBTYPE(DEPARE, 0)] = Polygon(QBrush(QColor(0x98, 0xc0, 0x64)));
	_polygons[SUBTYPE(DEPARE, 1)] = Polygon(QBrush(QColor(0xa0, 0xa0, 0xff)));
	_polygons[SUBTYPE(DEPARE, 2)] = Polygon(QBrush(QColor(0xb0, 0xb0, 0xff)));
	_polygons[SUBTYPE(DEPARE, 3)] = Polygon(QBrush(QColor(0xc0, 0xc0, 0xff)));
	_polygons[SUBTYPE(DEPARE, 4)] = Polygon(QBrush(QColor(0xc0, 0xd0, 0xff)));
	_polygons[SUBTYPE(DEPARE, 5)] = Polygon(QBrush(QColor(0xc0, 0xe0, 0xff)));
	_polygons[SUBTYPE(DEPARE, 6)] = Polygon(QBrush(QColor(0xff, 0xff, 0xff)));
	_polygons[TYPE(DMPGRD)] = Polygon(QBrush(QColor(0xa3, 0xa3, 0xa3),
	  Qt::Dense3Pattern));
	_polygons[TYPE(FAIRWY)] = Polygon(Qt::NoBrush, QPen(QColor(0x88, 0x88, 0x88),
	  1, Qt::DashDotDotLine));
	_polygons[TYPE(OBSTRN)] = Polygon(Qt::NoBrush, QPen(QColor(0, 0, 0), 1.5,
	  Qt::DotLine));
	_polygons[TYPE(UWTROC)] = Polygon(Qt::NoBrush, QPen(QColor(0, 0, 0), 1.5,
	  Qt::DotLine));
	_polygons[TYPE(PONTON)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)));
	_polygons[TYPE(I_PONTON)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)));
	_polygons[TYPE(HULKES)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)));
	_polygons[TYPE(I_HULKES)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)));
	_polygons[TYPE(DRYDOC)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)));
	_polygons[TYPE(SLCONS)] = Polygon(Qt::NoBrush,
	  QPen(QColor(0x33, 0x33, 0x33), 1.5, Qt::DashLine));
	_polygons[TYPE(I_SLCONS)] = Polygon(Qt::NoBrush,
	  QPen(QColor(0x33, 0x33, 0x33), 1.5, Qt::DashLine));
	_polygons[TYPE(LAKARE)] = Polygon(QBrush(QColor(0x9f, 0xc4, 0xe1)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(CANALS)] = Polygon(QBrush(QColor(0x9f, 0xc4, 0xe1)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(RIVERS)] = Polygon(QBrush(QColor(0x9f, 0xc4, 0xe1)));
	_polygons[TYPE(DYKCON)] = Polygon(QBrush(QColor(0x9f, 0xc4, 0xe1),
	  Qt::Dense4Pattern), QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(AIRARE)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)));
	_polygons[TYPE(TSEZNE)] = Polygon(QBrush(QColor(0xfc, 0xb4, 0xfc, 0x80)));
	_polygons[TYPE(DRGARE)] = Polygon(QBrush(QColor(0xa0, 0xa0, 0xff),
	  Qt::Dense4Pattern));
	_polygons[TYPE(UNSARE)] = Polygon(QBrush(QColor(0x99, 0x99, 0x99)));
	_polygons[SUBTYPE(RESARE, 9)] = Polygon(QBrush(QColor(0xff, 0x00, 0x00),
	  Qt::BDiagPattern));
	_polygons[SUBTYPE(RESARE, 2)] = Polygon(QImage(":/marine/noanchor-line.png"));
	_polygons[SUBTYPE(I_RESARE, 2)] = Polygon(QImage(":/marine/noanchor-line.png"));
	_polygons[SUBTYPE(RESARE, 17)] = Polygon(
	  QImage(":/marine/entry-prohibited-line.png"));
	_polygons[SUBTYPE(I_RESARE, 17)] = Polygon(
	  QImage(":/marine/entry-prohibited-line.png"));
	_polygons[SUBTYPE(RESARE, 12)] = Polygon(QImage(":/marine/safety-zone-line.png"));
	_polygons[SUBTYPE(I_RESARE, 12)] = Polygon(QImage(":/marine/safety-zone-line.png"));
	_polygons[SUBTYPE(RESARE, 1)] = Polygon(QImage(":/marine/safety-zone-line.png"));
	_polygons[SUBTYPE(ACHARE, 1)] = Polygon(QImage(":/marine/anchor-line.png"));
	_polygons[SUBTYPE(ACHARE, 2)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(ACHARE, 3)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(ACHARE, 4)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(ACHARE, 5)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(ACHARE, 6)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(ACHARE, 7)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(ACHARE, 8)] = Polygon(Qt::NoBrush,
	  QPen(QColor(0xeb, 0x49, 0xeb), 1, Qt::DashLine));
	_polygons[SUBTYPE(ACHARE, 9)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(I_ACHARE, 1)] = _polygons[SUBTYPE(ACHARE, 1)];
	_polygons[SUBTYPE(I_ACHARE, 2)] = _polygons[SUBTYPE(ACHARE, 2)];
	_polygons[SUBTYPE(I_ACHARE, 3)] = _polygons[SUBTYPE(ACHARE, 3)];
	_polygons[SUBTYPE(I_ACHARE, 4)] = _polygons[SUBTYPE(ACHARE, 4)];
	_polygons[SUBTYPE(I_ACHARE, 5)] = _polygons[SUBTYPE(ACHARE, 5)];
	_polygons[SUBTYPE(I_ACHARE, 6)] = _polygons[SUBTYPE(ACHARE, 6)];
	_polygons[SUBTYPE(I_ACHARE, 7)] = _polygons[SUBTYPE(ACHARE, 7)];
	_polygons[SUBTYPE(I_ACHARE, 8)] = _polygons[SUBTYPE(ACHARE, 8)];
	_polygons[SUBTYPE(I_ACHARE, 9)] = _polygons[SUBTYPE(ACHARE, 9)];
	_polygons[SUBTYPE(I_ACHARE, 10)] = _polygons[SUBTYPE(I_ACHARE, 1)];
	_polygons[SUBTYPE(I_ACHARE, 11)] = _polygons[SUBTYPE(I_ACHARE, 1)];
	_polygons[SUBTYPE(I_ACHARE, 12)] = _polygons[SUBTYPE(I_ACHARE, 1)];
	_polygons[TYPE(PRCARE)] = Polygon(QBrush(QColor(0xeb, 0x49, 0xeb),
	  Qt::BDiagPattern));
	_polygons[TYPE(DAMCON)] = Polygon(QBrush(QColor(0xd9, 0x8b, 0x21)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(DRYDOC)] = Polygon(QBrush(QColor(0xeb, 0xab, 0x54)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(PYLONS)] = Polygon(QBrush(QColor(0xa5, 0x81, 0x40)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(FLODOC)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(I_FLODOC)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33)),
	  QPen(QColor(0, 0, 0), 1));
	_polygons[TYPE(DWRTPT)] = Polygon(QImage(":/marine/dw-route-line.png"));
	_polygons[TYPE(MORFAC)] = Polygon(QBrush(QColor(0xe8, 0xe0, 0x64)),
	  QPen(QColor(0, 0, 0), 2));
	_polygons[TYPE(GATCON)] = Polygon(QBrush(QColor(0, 0, 0)));
	_polygons[TYPE(I_GATCON)] = Polygon(QBrush(QColor(0, 0, 0)));
	_polygons[TYPE(I_TERMNL)] = Polygon(QBrush(QColor(0xb8, 0xb0, 0x4b)),
	  QPen(QColor(0x96, 0x61, 0x18)));
	_polygons[TYPE(SILTNK)] = Polygon(QBrush(QColor(0xd9, 0x8b, 0x21)),
	  QPen(QColor(0x96, 0x61, 0x18), 2));
	_polygons[TYPE(LOKBSN)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33),
	  Qt::Dense7Pattern));
	_polygons[TYPE(I_LOKBSN)] = Polygon(QBrush(QColor(0x33, 0x33, 0x33),
	  Qt::Dense7Pattern));
	_polygons[TYPE(TUNNEL)] = Polygon(Qt::NoBrush, QPen(QColor(0xa5, 0x81, 0x40),
	  1.5, Qt::DashLine));
	_polygons[TYPE(CBLARE)] = Polygon(QImage(":/marine/cable-area-line.png"));
	_polygons[TYPE(PIPARE)] = Polygon(QImage(":/marine/pipeline-area-line.png"));
	_polygons[SUBTYPE(MARKUL, 3)] = Polygon(QImage(":/marine/fishing-farm-line.png"));
	_polygons[TYPE(BERTHS)] = Polygon(Qt::NoBrush, QPen(QColor(0xeb, 0x49, 0xeb),
	  1, Qt::DashLine));
	_polygons[TYPE(I_BERTHS)] = _polygons[TYPE(BERTHS)];
	_polygons[SUBTYPE(I_BERTHS, 6)] = _polygons[TYPE(BERTHS)];
	_polygons[TYPE(I_TRNBSN)] = Polygon(Qt::NoBrush, QPen(QColor(0xeb, 0x49, 0xeb),
	  1, Qt::DashLine));
	_polygons[TYPE(CONZNE)] = Polygon(Qt::NoBrush, QPen(QColor(0xeb, 0x49, 0xeb),
	  1, Qt::DashDotLine));

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
	  << TYPE(MORFAC) << TYPE(GATCON) << TYPE(I_GATCON) << TYPE(BERTHS)
	  << TYPE(I_BERTHS) << SUBTYPE(I_BERTHS, 6) << TYPE(DMPGRD) << TYPE(TSEZNE)
	  << TYPE(OBSTRN) << TYPE(UWTROC) << TYPE(DWRTPT) << SUBTYPE(ACHARE, 1)
	  << SUBTYPE(ACHARE, 2) << SUBTYPE(ACHARE, 3) << SUBTYPE(ACHARE, 4)
	  << SUBTYPE(ACHARE, 5) << SUBTYPE(ACHARE, 6) << SUBTYPE(ACHARE, 7)
	  << SUBTYPE(ACHARE, 8) << SUBTYPE(ACHARE, 9) << SUBTYPE(I_ACHARE, 1)
	  << SUBTYPE(I_ACHARE, 2) << SUBTYPE(I_ACHARE, 3) << SUBTYPE(I_ACHARE, 4)
	  << SUBTYPE(I_ACHARE, 5) << SUBTYPE(I_ACHARE, 6) << SUBTYPE(I_ACHARE, 7)
	  << SUBTYPE(I_ACHARE, 8) << SUBTYPE(I_ACHARE, 9) << SUBTYPE(I_ACHARE, 10)
	  << SUBTYPE(I_ACHARE, 11) << SUBTYPE(I_ACHARE, 12) << SUBTYPE(RESARE, 9)
	  << SUBTYPE(RESARE, 2) << SUBTYPE(I_RESARE, 2) << SUBTYPE(RESARE, 17)
	  << SUBTYPE(I_RESARE, 17) << SUBTYPE(RESARE, 12) << SUBTYPE(I_RESARE, 12)
	  << SUBTYPE(RESARE, 1) << TYPE(CBLARE) << TYPE(PIPARE) << TYPE(PRCARE)
	  << TYPE(I_TRNBSN) << SUBTYPE(MARKUL, 3) << TYPE(CONZNE);
}

void Style::lineStyle(qreal ratio)
{
	_lines[TYPE(BUISGL)] = Line(QPen(QColor(0x96, 0x61, 0x18), 1.5));
	_lines[TYPE(DEPCNT)] = Line(QPen(QColor(0x65, 0x9a, 0xef), 1, Qt::SolidLine));
	_lines[TYPE(DEPCNT)].setTextColor(QColor(0x55, 0x8a, 0xdf));
	_lines[TYPE(DEPCNT)].setTextFontSize(Small);
	_lines[TYPE(CBLOHD)] = Line(QImage(":/marine/cable-line.png"));
	_lines[TYPE(I_CBLOHD)] = Line(QImage(":/marine/cable-line.png"));
	_lines[TYPE(BRIDGE)] = Line(QPen(QColor(0xa5, 0x81, 0x40), 3, Qt::SolidLine));
	_lines[TYPE(I_BRIDGE)] = Line(QPen(QColor(0xa5, 0x81, 0x40), 3, Qt::SolidLine));
	_lines[TYPE(CBLSUB)] = Line(QImage(":/marine/cable.png"));
	_lines[TYPE(CBLSUB)].setTextFontSize(Small);
	_lines[TYPE(PIPSOL)] = Line(QImage(":/marine/pipeline.png"));
	_lines[TYPE(PIPSOL)].setTextFontSize(Small);
	_lines[TYPE(NAVLNE)] = Line(QPen(QColor(0, 0, 0), 1, Qt::DashLine));
	_lines[TYPE(COALNE)] = Line(QPen(QColor(0, 0, 0), 1, Qt::SolidLine));
	_lines[TYPE(SLCONS)] = Line(QPen(QColor(0, 0, 0), 2, Qt::SolidLine));
	_lines[TYPE(I_SLCONS)] = Line(QPen(QColor(0, 0, 0), 2, Qt::SolidLine));
	_lines[TYPE(PONTON)] = Line(QPen(QColor(0x33, 0x33, 0x33), 1, Qt::SolidLine));
	_lines[TYPE(DYKCON)] = Line(QPen(QColor(0x33, 0x33, 0x33), 2, Qt::SolidLine));
	_lines[TYPE(RIVERS)] = Line(QPen(QColor(0, 0, 0), 1, Qt::SolidLine));
	_lines[TYPE(TSSBND)] = Line(QPen(QColor(0xeb, 0x49, 0xeb), 2, Qt::DashLine));
	_lines[TYPE(LNDELV)] = Line(QPen(QColor(0x99, 0x94, 0x40), 1, Qt::SolidLine));
	_lines[TYPE(LNDELV)].setTextColor(QColor(0x79, 0x74, 0x20));
	_lines[TYPE(LNDELV)].setTextFontSize(Small);
	_lines[TYPE(SLOTOP)] = Line(QPen(QColor(0x79, 0x74, 0x20), 1, Qt::SolidLine));
	_lines[TYPE(OBSTRN)] = Line(QPen(QColor(0, 0, 0), 1.5, Qt::DotLine));
	_lines[TYPE(FERYRT)] = Line(QImage(":/marine/ferry-line.png"));
	_lines[TYPE(FERYRT)].setTextFontSize(Small);
	_lines[TYPE(I_FERYRT)] = Line(QImage(":/marine/ferry-line.png"));
	_lines[TYPE(I_FERYRT)].setTextFontSize(Small);
	_lines[TYPE(RAILWY)] = Line(railroad(ratio));
	_lines[TYPE(ROADWY)] = Line(QPen(QColor(0, 0, 0), 2, Qt::SolidLine));
	_lines[TYPE(GATCON)] = Line(QPen(QColor(0, 0, 0), 2, Qt::SolidLine));
	_lines[TYPE(I_GATCON)] = Line(QPen(QColor(0, 0, 0), 2, Qt::SolidLine));
	_lines[TYPE(TSELNE)] = Line(QPen(QColor(0xfc, 0xb4, 0xfc, 0x80), 4,
	  Qt::SolidLine));
	_lines[TYPE(I_WTWAXS)] = Line(QPen(QColor(0, 0, 0), 0, Qt::DashLine));
	_lines[SUBTYPE(RECTRC, 1)] = Line(QPen(QColor(0, 0, 0), 0, Qt::SolidLine));
	_lines[SUBTYPE(RECTRC, 2)] = Line(QPen(QColor(0, 0, 0), 0, Qt::DashLine));
	_lines[SUBTYPE(RCRTCL, 1)] = Line(QPen(QColor(0xeb, 0x49, 0xeb), 0,
	  Qt::SolidLine));
	_lines[SUBTYPE(RCRTCL, 2)] = Line(QPen(QColor(0xeb, 0x49, 0xeb), 0,
	  Qt::DashLine));
	_lines[TYPE(FAIRWY)] = Line(QPen(QColor(0x88, 0x88, 0x88), 1,
	  Qt::DashDotDotLine));
	_lines[TYPE(BERTHS)] = Line(QPen(QColor(0x33, 0x33, 0x33), 2));
	_lines[TYPE(I_BERTHS)] = Line(QPen(QColor(0x33, 0x33, 0x33), 2));
	_lines[TYPE(FNCLNE)] = Line(QImage(":/marine/fence-line.png"));
	_lines[TYPE(CONVYR)] = Line(QImage(":/marine/conveyor-line.png"));
	_lines[TYPE(PIPOHD)] = Line(QImage(":/marine/pipeline-overhead.png"));
	_lines[TYPE(I_PIPOHD)] = Line(QImage(":/marine/pipeline-overhead.png"));
	_lines[TYPE(CANALS)] = Line(QPen(QColor(0x9f, 0xc4, 0xe1), 2));
}

void Style::pointStyle(qreal ratio)
{
	_points[SUBTYPE(BUAARE, 1)].setTextFontSize(Large);
	_points[SUBTYPE(BUAARE, 5)].setTextFontSize(Large);
	_points[SUBTYPE(BUAARE, 4)].setTextFontSize(Large);
	_points[SUBTYPE(BUAARE, 2)].setTextFontSize(Small);
	_points[SUBTYPE(BUAARE, 6)].setTextFontSize(Small);
	_points[TYPE(SOUNDG)].setTextFontSize(Small);
	_points[TYPE(SOUNDG)].setHaloColor(QColor());
	_points[TYPE(BOYCAR)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(BOYINB)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(BOYISD)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(BOYLAT)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(I_BOYLAT)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(BOYSAW)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(BOYSPP)] = Point(QImage(":/marine/buoy.png"), Small,
	  QPoint(6, -6));
	_points[TYPE(BCNCAR)] = Point(QImage(":/marine/beacon.png"), Small,
	  QPoint(0, -8));
	_points[TYPE(BCNISD)] = Point(QImage(":/marine/beacon.png"), Small,
	  QPoint(0, -8));
	_points[TYPE(BCNLAT)] = Point(QImage(":/marine/beacon.png"), Small,
	  QPoint(0, -8));
	_points[TYPE(I_BCNLAT)] = Point(QImage(":/marine/beacon.png"), Small,
	  QPoint(0, -8));
	_points[TYPE(BCNSAW)] = Point(QImage(":/marine/beacon.png"), Small,
	  QPoint(0, -8));
	_points[TYPE(BCNSPP)] = Point(QImage(":/marine/beacon.png"), Small,
	  QPoint(0, -8));
	_points[SUBTYPE(LNDMRK, 3)] = Point(QImage(":/marine/chimney.png"),
	  Small, QPoint(0, -11));
	_points[SUBTYPE(LNDMRK, 5)] = Point(QImage(":/marine/flagstaff.png"),
	  Small, QPoint(2, -11));
	_points[SUBTYPE(LNDMRK, 7)] = Point(QImage(":/marine/pylon.png"), Small);
	_points[SUBTYPE(LNDMRK, 9)] = Point(QImage(":/marine/monument.png"), Small,
	 QPoint(0, -7));
	_points[SUBTYPE(LNDMRK, 10)] = Point(QImage(":/marine/pylon.png"), Small);
	_points[SUBTYPE(LNDMRK, 15)] = Point(QImage(":/marine/dome.png"), Small,
	 QPoint(0, -5));
	_points[SUBTYPE(LNDMRK, 17)] = Point(QImage(":/marine/tower.png"), Small,
	  QPoint(0, -11));
	_points[SUBTYPE(LNDMRK, 18)] = Point(QImage(":/marine/windmill.png"), Small);
	_points[SUBTYPE(LNDMRK, 19)] = Point(QImage(":/marine/windmotor.png"),
	  Small, QPoint(0, -11));
	_points[SUBTYPE(LNDMRK, 20)] = Point(QImage(":/marine/church.png"), Small);
	_points[TYPE(LNDELV)] = Point(QImage(":/marine/triangulation-point.png"));
	_points[TYPE(OBSTRN)] = Point(QImage(":/marine/obstruction.png"), Small);
	_points[SUBTYPE(WRECKS, 0)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 1)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 2)] = Point(QImage(":/marine/wreck-dangerous.png"),
	  Small);
	_points[SUBTYPE(WRECKS, 3)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 4)] = Point(QImage(":/marine/wreck.png"), Small);
	_points[SUBTYPE(WRECKS, 5)] = Point(QImage(":/marine/wreck-exposed.png"),
	  Small, QPoint(0, -4));
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
	_points[SUBTYPE(HRBFAC, 9)] = Point(QImage(":/marine/shipyard.png"));
	_points[SUBTYPE(I_HRBFAC, 4)] = Point(QImage(":/marine/fishing-harbor.png"));
	_points[SUBTYPE(I_HRBFAC, 5)] = Point(QImage(":/marine/yacht-harbor.png"));
	_points[SUBTYPE(I_HRBFAC, 9)] = Point(QImage(":/marine/shipyard.png"));
	_points[TYPE(ACHBRT)] = Point(QImage(":/marine/anchorage.png"));
	_points[TYPE(I_ACHBRT)] = Point(QImage(":/marine/anchorage.png"));
	_points[TYPE(OFSPLF)] = Point(QImage(":/marine/platform.png"));
	_points[TYPE(PILPNT)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 1)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 2)] = Point(QImage(":/marine/deviation-dolphin.png"),
	  Small, QPoint(0, -6));
	_points[SUBTYPE(MORFAC, 3)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 5)] = Point(QImage(":/marine/pile.png"), Small);
	_points[SUBTYPE(MORFAC, 7)] = Point(QImage(":/marine/mooring-buoy.png"),
	  Small, QPoint(0, -5));
	_points[TYPE(CRANES)] = Point(QImage(":/marine/crane.png"), Small,
	  QPoint(0, -5));
	_points[TYPE(I_CRANES)] = Point(QImage(":/marine/crane.png"), Small,
	  QPoint(0, -5));
	_points[SUBTYPE(I_DISMAR, 1)] = Point(QImage(":/marine/distance-mark.png"),
	  Small);
	_points[SUBTYPE(I_DISMAR, 1)].setTextColor(QColor(0xff, 0xff, 0xff));
	_points[SUBTYPE(I_DISMAR, 1)].setHaloColor(QColor());
	_points[SUBTYPE(I_DISMAR, 2)] = Point(QImage(":/marine/distance-mark-land.png"),
	  Small);
	_points[SUBTYPE(I_DISMAR, 2)].setHaloColor(QColor());
	_points[SUBTYPE(I_DISMAR, 3)] = _points[SUBTYPE(I_DISMAR, 2)];
	_points[SUBTYPE(I_DISMAR, 4)] = _points[SUBTYPE(I_DISMAR, 2)];
	_points[TYPE(CGUSTA)] = Point(QImage(":/marine/coast-guard.png"));
	_points[TYPE(RSCSTA)] = Point(QImage(":/marine/rescue-station.png"));
	_points[TYPE(RDOSTA)] = Point(QImage(":/marine/radio.png"));
	_points[TYPE(RADSTA)] = Point(QImage(":/marine/radar.png"));
	_points[TYPE(RTPBCN)] = Point(QImage(":/marine/radar-transponder.png"));
	_points[SUBTYPE(SILTNK, 0)] = Point(QImage(":/marine/silo.png"));
	_points[SUBTYPE(SILTNK, 1)] = Point(QImage(":/marine/silo.png"));
	_points[SUBTYPE(SILTNK, 2)] = Point(QImage(":/marine/tank.png"));
	_points[SUBTYPE(SILTNK, 3)] = Point(QImage(":/marine/silo.png"));
	_points[SUBTYPE(SILTNK, 4)] = Point(QImage(":/marine/silo.png"));
	_points[TYPE(I_TRNBSN)] = Point(QImage(":/marine/turning-basin.png"));
	_points[TYPE(I_TRNBSN)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[TYPE(I_WTWGAG)] = Point(QImage(":/marine/gauge.png"), Small);
	_points[SUBTYPE(RDOCAL, 1)] = Point(QImage(":/marine/radio-call.png"));
	_points[SUBTYPE(RDOCAL, 2)] = Point(QImage(":/marine/radio-call.png"));
	_points[SUBTYPE(RDOCAL, 3)] = Point(QImage(":/marine/radio-call.png"));
	_points[SUBTYPE(RDOCAL, 4)] = Point(QImage(":/marine/radio-call-2w.png"));
	_points[SUBTYPE(RDOCAL, 1)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(RDOCAL, 2)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(RDOCAL, 3)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(RDOCAL, 4)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(I_RDOCAL, 1)] = Point(QImage(":/marine/radio-call.png"));
	_points[SUBTYPE(I_RDOCAL, 2)] = Point(QImage(":/marine/radio-call.png"));
	_points[SUBTYPE(I_RDOCAL, 3)] = Point(QImage(":/marine/radio-call.png"));
	_points[SUBTYPE(I_RDOCAL, 4)] = Point(QImage(":/marine/radio-call-2w.png"));
	_points[SUBTYPE(I_RDOCAL, 1)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(I_RDOCAL, 2)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(I_RDOCAL, 3)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[SUBTYPE(I_RDOCAL, 4)].setTextColor(QColor(0xeb, 0x49, 0xeb));
	_points[TYPE(PYLONS)] = Point(QImage(":/marine/pylon.png"));
	_points[SUBTYPE(WATTUR, 1)] = Point(QImage(":/marine/breakers.png"));
	_points[SUBTYPE(WATTUR, 2)] = Point(QImage(":/marine/eddies.png"));
	_points[SUBTYPE(WATTUR, 3)] = Point(QImage(":/marine/overfalls.png"));
	_points[SUBTYPE(WATTUR, 4)] = Point(QImage(":/marine/overfalls.png"));
	_points[TYPE(PILBOP)] = Point(QImage(":/marine/boarding-place.png"));
	_points[TYPE(SISTAT)] = Point(QImage(":/marine/pylon.png"));
	_points[TYPE(SLCONS)] = Point(QImage(":/marine/construction.png"), Small);
	_points[TYPE(CURENT)] = Point(QImage(":/marine/current.png"));
	_points[SUBTYPE(WEDKLP, 0)] = Point(QImage(":/marine/kelp.png"));
	_points[SUBTYPE(WEDKLP, 1)] = Point(QImage(":/marine/kelp.png"));
	_points[TYPE(SEAARE)].setHaloColor(QColor());
	_points[TYPE(RADRFL)] = Point(QImage(":/marine/radar-reflector.png"));

	_points[SUBTYPE(I_BERTHS, 6)] = Point(QImage(":/marine/fleeting-area.png"),
	  Small);
	_points[SUBTYPE(ACHARE, 2)] = Point(QImage(":/marine/dw-anchorage.png"),
	  Small);
	_points[SUBTYPE(ACHARE, 3)] = Point(QImage(":/marine/tanker-anchorage.png"),
	  Small);
	_points[SUBTYPE(ACHARE, 9)] = Point(QImage(":/marine/24h-anchorage.png"),
	  Small);
	_points[SUBTYPE(I_ACHARE, 2)] = _points[SUBTYPE(ACHARE, 2)];
	_points[SUBTYPE(I_ACHARE, 3)] = _points[SUBTYPE(ACHARE, 3)];
	_points[SUBTYPE(I_ACHARE, 9)] = _points[SUBTYPE(ACHARE, 9)];

	_points[SUBTYPE(SMCFAC, 7)] = Point(svg2img(":/POI/restaurant-11.svg",
	  ratio), Small);
	_points[SUBTYPE(SMCFAC, 11)] = Point(svg2img(":/POI/pharmacy-11.svg",
	  ratio), Small);
	_points[SUBTYPE(SMCFAC, 12)] = Point(svg2img(":/POI/drinking-water-11.svg",
	  ratio), Small);
	_points[SUBTYPE(SMCFAC, 13)] = Point(svg2img(":/POI/fuel-11.svg", ratio),
	  Small);
	_points[SUBTYPE(SMCFAC, 18)] = Point(svg2img(":/POI/toilet-11.svg", ratio),
	  Small);
	_points[SUBTYPE(SMCFAC, 20)] = Point(svg2img(":/POI/telephone-11.svg",
	  ratio), Small);
	_points[SUBTYPE(SMCFAC, 22)] = Point(svg2img(":/POI/parking-11.svg", ratio),
	  Small);
	_points[SUBTYPE(SMCFAC, 25)] = Point(svg2img(":/POI/campsite-11.svg",
	  ratio), Small);
	_points[TYPE(BUISGL)] = Point(QImage(":/marine/building.png"), Small);
	_points[SUBTYPE(BUISGL, 2)] = Point(svg2img(":/POI/harbor-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 5)] = Point(svg2img(":/POI/hospital-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 6)] = Point(svg2img(":/POI/post-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 7)] = Point(svg2img(":/POI/lodging-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 8)] = Point(svg2img(":/POI/rail-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 9)] = Point(svg2img(":/POI/police-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 13)] = Point(svg2img(":/POI/bank-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 19)] = Point(svg2img(":/POI/school-11.svg", ratio),
	  Small);
	_points[SUBTYPE(BUISGL, 20)] = Point(svg2img(":/POI/religious-christian-11.svg",
	  ratio), Small);
	_points[SUBTYPE(BUISGL, 22)] = Point(svg2img(":/POI/religious-jewish-11.svg",
	  ratio), Small);
	_points[SUBTYPE(BUISGL, 26)] = Point(svg2img(":/POI/religious-muslim-11.svg",
	  ratio), Small);
	_points[SUBTYPE(BUISGL, 33)] = Point(QImage(":/marine/pylon.png"), Small);
	_points[SUBTYPE(BUISGL, 42)] = Point(svg2img(":/POI/bus-11.svg", ratio),
	  Small);
}

Style::Style(qreal ratio)
{
	_light = QImage(":/marine/light.png");
	_lightOffset = QPoint(11, 11);
	_signal = QImage(":/marine/fog-signal.png");
	_signalOffset = QPoint(-9, 9);

	_large = pixelSizeFont(16);
	_normal = pixelSizeFont(12);
	_small = pixelSizeFont(10);

	polygonStyle();
	lineStyle(ratio);
	pointStyle(ratio);
}

const Style::Line &Style::line(uint type) const
{
	static Line null;

	QMap<uint, Line>::const_iterator it(_lines.find(type));
	return (it == _lines.constEnd()) ? null : *it;
}

const Style::Polygon &Style::polygon(uint type) const
{
	static Polygon null;

	QMap<uint, Polygon>::const_iterator it(_polygons.find(type));
	return (it == _polygons.constEnd()) ? null : *it;
}

const Style::Point &Style::point(uint type) const
{
	static Point null;

	QMap<uint, Point>::const_iterator it(_points.find(type));
	return (it == _points.constEnd()) ? null : *it;
}

const QFont *Style::font(Style::FontSize size) const
{
	switch (size) {
		case Style::None:
			return 0;
		case Style::Large:
			return &_large;
		case Style::Small:
			return &_small;
		default:
			return &_normal;
	}
}
