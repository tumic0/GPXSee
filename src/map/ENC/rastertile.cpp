#include <QtMath>
#include <QPainter>
#include "common/linec.h"
#include "map/bitmapline.h"
#include "map/textpathitem.h"
#include "map/textpointitem.h"
#include "map/rectd.h"
#include "objects.h"
#include "style.h"
#include "rastertile.h"

using namespace ENC;

#define TEXT_EXTENT 160
#define TSSLPT_SIZE  0.005 /* ll */

typedef QSet<Coordinates> PointSet;

static const float C1 = 0.866025f; /* sqrt(3)/2 */
static const QColor tsslptPen = QColor(0xeb, 0x49, 0xeb);
static const QColor tsslptBrush = QColor(0xeb, 0x49, 0xeb, 0x80);

static double area(const QVector<Coordinates> &polygon)
{
	double area = 0;

	for (int i = 0; i < polygon.size() - 1; i++) {
		const Coordinates &pi = polygon.at(i);
		const Coordinates &pj = polygon.at(i+1);
		area += pi.lon() * pj.lat();
		area -= pi.lat() * pj.lon();
	}
	area /= 2.0;

	return area;
}

static Coordinates centroid(const QVector<Coordinates> &polygon)
{
	Q_ASSERT(polygon.size() > 3);
	Q_ASSERT(polygon.first() == polygon.last());

	double cx = 0, cy = 0;
	double factor = 1.0 / (6.0 * area(polygon));

	for (int i = 0; i < polygon.size() - 1; i++) {
		const Coordinates &pi = polygon.at(i);
		const Coordinates &pj = polygon.at(i+1);
		double f = (pi.lon() * pj.lat() - pj.lon() * pi.lat());
		cx += (pi.lon() + pj.lon()) * f;
		cy += (pi.lat() + pj.lat()) * f;
	}

	return Coordinates(cx * factor, cy * factor);
}

static double angle(uint type, const QVariant &param)
{
	uint bt = type>>16;

	return (bt == RDOCAL || bt == I_RDOCAL || bt == CURENT)
	  ? 90 + param.toDouble() : NAN;
}

static bool showLabel(const QImage *img, const Range &range, int zoom, int type)
{
	if (type>>16 == I_DISMAR)
		return true;

	int limit = (!range.size())
	  ? range.min() : range.min() + (range.size() + 1) / 2;
	if ((img || (type>>16 == SOUNDG)) && (zoom < limit))
		return false;

	return true;
}

QPainterPath RasterTile::painterPath(const Polygon &polygon) const
{
	QPainterPath path;

	for (int i = 0; i < polygon.size(); i++) {
		const QVector<Coordinates> &subpath = polygon.at(i);

		QVector<QPointF> p;
		p.reserve(subpath.size());

		for (int j = 0; j < subpath.size(); j++) {
			const Coordinates &c = subpath.at(j);
			if (!c.isNull())
				p.append(ll2xy(c));
		}
		path.addPolygon(p);
	}

	return path;
}

QPolygonF RasterTile::polyline(const QVector<Coordinates> &path) const
{
	QPolygonF polygon;
	polygon.reserve(path.size());

	for (int i = 0; i < path.size(); i++)
		polygon.append(ll2xy(path.at(i)));

	return polygon;
}

QVector<QPolygonF> RasterTile::polylineM(const QVector<Coordinates> &path) const
{
	QVector<QPolygonF> polys;
	QPolygonF polygon;
	bool mask = false;

	polygon.reserve(path.size());

	for (int i = 0; i < path.size(); i++) {
		const Coordinates &c = path.at(i);

		if (c.isNull()) {
			if (mask)
				mask = false;
			else {
				polys.append(polygon);
				polygon.clear();
				mask = true;
			}
		} else if (!mask)
			polygon.append(ll2xy(c));
	}

	if (!polygon.isEmpty())
		polys.append(polygon);

	return polys;
}

QPolygonF RasterTile::tsslptArrow(const Coordinates &c, qreal angle) const
{
	Coordinates t[3], r[4];
	QPolygonF polygon;

	t[0] = c;
	t[1] = Coordinates(t[0].lon() - qCos(angle - M_PI/3) * TSSLPT_SIZE,
	  t[0].lat() - qSin(angle - M_PI/3) * TSSLPT_SIZE);
	t[2] = Coordinates(t[0].lon() - qCos(angle - M_PI + M_PI/3) * TSSLPT_SIZE,
	  t[0].lat() - qSin(angle - M_PI + M_PI/3) * TSSLPT_SIZE);

	LineC l(t[1], t[2]);
	r[0] = l.pointAt(0.25);
	r[1] = l.pointAt(0.75);
	r[2] = Coordinates(r[0].lon() - C1 * TSSLPT_SIZE * qCos(angle - M_PI/2),
	  r[0].lat() - C1 * TSSLPT_SIZE * qSin(angle - M_PI/2));
	r[3] = Coordinates(r[1].lon() - C1 * TSSLPT_SIZE * qCos(angle - M_PI/2),
	  r[1].lat() - C1 * TSSLPT_SIZE * qSin(angle - M_PI/2));

	polygon << ll2xy(t[0]) << ll2xy(t[2]) << ll2xy(r[1]) << ll2xy(r[3])
	  << ll2xy(r[2]) << ll2xy(r[0]) << ll2xy(t[1]);

	return polygon;
}

void RasterTile::drawArrows(QPainter *painter,
  const QList<MapData::Poly> &polygons)
{
	for (int i = 0; i < polygons.size(); i++) {
		const MapData::Poly &poly = polygons.at(i);

		if (poly.type()>>16 == TSSLPT) {
			QPolygonF polygon(tsslptArrow(centroid(poly.path().first()),
			  deg2rad(180 - poly.param().toDouble())));

			painter->setPen(QPen(tsslptPen, 1));
			painter->setBrush(QBrush(tsslptBrush));
			painter->drawPolygon(polygon);
		}
	}
}

void RasterTile::drawPolygons(QPainter *painter,
  const QList<MapData::Poly> &polygons)
{
	for (int n = 0; n < _style->drawOrder().size(); n++) {
		for (int i = 0; i < polygons.size(); i++) {
			const MapData::Poly &poly = polygons.at(i);
			if (poly.type() != _style->drawOrder().at(n))
				continue;
			const Style::Polygon &style = _style->polygon(poly.type());

			if (!style.img().isNull()) {
				for (int i = 0; i < poly.path().size(); i++)
					BitmapLine::draw(painter, polylineM(poly.path().at(i)),
					  style.img());
			} else {
				if (style.brush() != Qt::NoBrush) {
					painter->setPen(Qt::NoPen);
					painter->setBrush(style.brush());
					painter->drawPath(painterPath(poly.path()));
				}
				if (style.pen() != Qt::NoPen) {
					painter->setPen(style.pen());
					for (int i = 0; i < poly.path().size(); i++) {
						QVector<QPolygonF> outline(polylineM(poly.path().at(i)));
						for (int j = 0; j < outline.size(); j++)
							painter->drawPolyline(outline.at(j));
					}
				}
			}
		}
	}
}

void RasterTile::drawLines(QPainter *painter, const QList<MapData::Line> &lines)
{
	painter->setBrush(Qt::NoBrush);

	for (int i = 0; i < lines.size(); i++) {
		const MapData::Line &line = lines.at(i);
		const Style::Line &style = _style->line(line.type());

		if (!style.img().isNull()) {
			BitmapLine::draw(painter, polyline(line.path()), style.img());
		} else if (style.pen() != Qt::NoPen) {
			painter->setPen(style.pen());
			painter->drawPolyline(polyline(line.path()));
		}
	}
}

void RasterTile::drawTextItems(QPainter *painter,
  const QList<TextItem*> &textItems)
{
	for (int i = 0; i < textItems.size(); i++)
		textItems.at(i)->paint(painter);
}

void RasterTile::processPolygons(const QList<MapData::Poly> &polygons,
  QList<TextItem*> &textItems)
{
	for (int i = 0; i < polygons.size(); i++) {
		const MapData::Poly &poly = polygons.at(i);
		uint type = poly.type()>>16;

		if (!(type == HRBFAC || type == I_TRNBSN
		  || poly.type() == SUBTYPE(I_BERTHS, 6)))
			continue;
		const Style::Point &style = _style->point(poly.type());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		if (!img)
			continue;

		TextPointItem *item = new TextPointItem(
		  ll2xy(centroid(poly.path().first())).toPoint(),
		  0, 0, img, 0, 0, 0, 0);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processPoints(QList<MapData::Point> &points,
  QList<TextItem*> &textItems, QList<TextItem*> &lights)
{
	PointSet lightsSet, signalsSet;
	int i;

	std::sort(points.begin(), points.end());

	/* Lights & Signals */
	for (i = 0; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		if (point.type()>>16 == LIGHTS)
			lightsSet.insert(point.pos());
		else if (point.type()>>16 == FOGSIG)
			signalsSet.insert(point.pos());
		else
			break;
	}

	/* Everything else */
	for ( ; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		QPoint pos(ll2xy(point.pos()).toPoint());
		const Style::Point &style = _style->point(point.type());

		const QString *label = point.label().isEmpty() ? 0 : &(point.label());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *fnt = showLabel(img, _zoomRange, _zoom, point.type())
		  ? _style->font(style.textFontSize()) : 0;
		const QColor *color = &style.textColor();
		const QColor *hColor = style.haloColor().isValid()
		  ? &style.haloColor() : 0;
		double rotate = angle(point.type(), point.param());

		if ((!label || !fnt) && !img)
			continue;

		QPoint offset = img ? style.offset() : QPoint(0, 0);

		TextPointItem *item = new TextPointItem(pos + offset, label, fnt, img,
		  color, hColor, 0, 2, rotate);
		if (item->isValid() && !item->collides(textItems)) {
			textItems.append(item);
			if (lightsSet.contains(point.pos()))
				lights.append(new TextPointItem(pos + _style->lightOffset(),
				  0, 0, _style->light(), 0, 0, 0, 0));
			if (signalsSet.contains(point.pos()))
				lights.append(new TextPointItem(pos + _style->signalOffset(),
				  0, 0, _style->signal(), 0, 0, 0, 0));
		} else
			delete item;
	}
}

void RasterTile::processLines(const QList<MapData::Line> &lines,
  QList<TextItem*> &textItems)
{
	for (int i = 0; i < lines.size(); i++) {
		const MapData::Line &line = lines.at(i);
		const Style::Line &style = _style->line(line.type());

		if (style.img().isNull() && style.pen() == Qt::NoPen)
			continue;
		if (line.label().isEmpty() || style.textFontSize() == Style::None)
			continue;

		const QFont *fnt = _style->font(style.textFontSize());
		const QColor *color = &style.textColor();

		TextPathItem *item = new TextPathItem(polyline(line.path()),
		  &line.label(), _rect, fnt, color, 0);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::fetchData(QList<MapData::Poly> &polygons,
  QList<MapData::Line> &lines, QList<MapData::Point> &points)
{
	QPoint ttl(_rect.topLeft());

	QRectF polyRect(ttl, QPointF(ttl.x() + _rect.width(), ttl.y()
	  + _rect.height()));
	RectD polyRectD(_transform.img2proj(polyRect.topLeft()),
	  _transform.img2proj(polyRect.bottomRight()));
	RectC polyRectC(polyRectD.toRectC(_proj, 20));
	QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT, ttl.y() - TEXT_EXTENT),
	  QPointF(ttl.x() + _rect.width() + TEXT_EXTENT, ttl.y() + _rect.height()
	  + TEXT_EXTENT));
	RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
	  _transform.img2proj(pointRect.bottomRight()));
	RectC pointRectC(pointRectD.toRectC(_proj, 20));

	if (_map) {
		_map->lines(polyRectC, &lines);
		_map->polygons(polyRectC, &polygons);
		_map->points(pointRectC, &points);
	} else {
		_atlas->polys(polyRectC, &polygons, &lines);
		_atlas->points(pointRectC, &points);
	}
}

void RasterTile::render()
{
	QImage img(_rect.width() * _ratio, _rect.height() * _ratio,
	  QImage::Format_ARGB32_Premultiplied);
	QList<MapData::Line> lines;
	QList<MapData::Poly> polygons;
	QList<MapData::Point> points;
	QList<TextItem*> textItems, lights;

	img.setDevicePixelRatio(_ratio);
	img.fill(Qt::transparent);

	fetchData(polygons, lines, points);

	processPolygons(polygons, textItems);
	processPoints(points, textItems, lights);
	processLines(lines, textItems);

	QPainter painter(&img);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawPolygons(&painter, polygons);
	drawLines(&painter, lines);
	drawArrows(&painter, polygons);

	drawTextItems(&painter, lights);
	drawTextItems(&painter, textItems);

	qDeleteAll(textItems);
	qDeleteAll(lights);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(_rect);

	_pixmap.convertFromImage(img);
}
