#include <QtMath>
#include <QPainter>
#include "map/bitmapline.h"
#include "map/textpathitem.h"
#include "map/textpointitem.h"
#include "map/rectd.h"
#include "objects.h"
#include "attributes.h"
#include "style.h"
#include "rastertile.h"

using namespace ENC;

#define TEXT_EXTENT 160
#define TSSLPT_SIZE 24
#define RANGE_FACTOR 4
#define MAJOR_RANGE 10

static const float C1 = 0.866025f; /* sqrt(3)/2 */
static const QColor tsslptPen = QColor(0xeb, 0x49, 0xeb);
static const QColor tsslptBrush = QColor(0xeb, 0x49, 0xeb, 0x80);

static double angle(uint type, const QVariant &param)
{
	uint bt = type>>16;

	return (bt == RDOCAL || bt == I_RDOCAL || bt == CURENT)
	  ? 90 + param.toDouble() : NAN;
}

bool RasterTile::showLabel(const QImage *img, int type) const
{
	if (type>>16 == I_DISMAR)
		return true;

	int limit = (!_zoomRange.size())
	  ? _zoomRange.min() : _zoomRange.min() + (_zoomRange.size() + 1) / 2;
	if ((img || (type>>16 == SOUNDG)) && (_zoom < limit))
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

QPolygonF RasterTile::tsslptArrow(const QPointF &p, qreal angle) const
{
	QPointF t[3], r[4];
	QPolygonF polygon;

	t[0] = p;
	t[1] = QPointF(t[0].x() - qCos(angle - M_PI/3) * TSSLPT_SIZE,
	  t[0].y() - qSin(angle - M_PI/3) * TSSLPT_SIZE);
	t[2] = QPointF(t[0].x() - qCos(angle - M_PI + M_PI/3) * TSSLPT_SIZE,
	  t[0].y() - qSin(angle - M_PI + M_PI/3) * TSSLPT_SIZE);

	QLineF l(t[1], t[2]);
	r[0] = l.pointAt(0.25);
	r[1] = l.pointAt(0.75);
	r[2] = QPointF(r[0].x() - C1 * TSSLPT_SIZE * qCos(angle - M_PI/2),
	  r[0].y() - C1 * TSSLPT_SIZE * qSin(angle - M_PI/2));
	r[3] = QPointF(r[1].x() - C1 * TSSLPT_SIZE * qCos(angle - M_PI/2),
	  r[1].y() - C1 * TSSLPT_SIZE * qSin(angle - M_PI/2));

	polygon << t[0] << t[2] << r[1] << r[3] << r[2] << r[0] << t[1];

	return polygon;
}

static void drawArrow(QPainter *painter, const QPolygonF &polygon, uint type)
{
	if (type>>16 == RCTLPT) {
		painter->setPen(QPen(tsslptPen, 1, Qt::DashLine));
		painter->setBrush(Qt::NoBrush);
	} else {
		painter->setPen(QPen(tsslptPen, 1));
		painter->setBrush(QBrush(tsslptBrush));
	}
	painter->drawPolygon(polygon);
}

void RasterTile::drawArrows(QPainter *painter,
  const QList<Data::Point> &points) const
{
	for (int i = 0; i < points.size(); i++) {
		const Data::Point &point = points.at(i);

		if (point.type()>>16 == TSSLPT || point.type()>>16 == RCTLPT) {
			QPolygonF polygon(tsslptArrow(ll2xy(point.pos()),
			  deg2rad(point.attributes().value(ORIENT).toDouble())));
			drawArrow(painter, polygon, point.type());
		}
	}
}

void RasterTile::drawPolygons(QPainter *painter,
  const QList<Data::Poly> &polygons) const
{
	for (int n = 0; n < _style->drawOrder().size(); n++) {
		for (int i = 0; i < polygons.size(); i++) {
			const Data::Poly &poly = polygons.at(i);
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
					QPainterPath path(painterPath(poly.path()));
					if (poly.type() == TYPE(DRGARE)) {
						painter->setBrush(Qt::white);
						painter->drawPath(path);
					}
					painter->setBrush(style.brush());
					painter->drawPath(path);
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

void RasterTile::drawLines(QPainter *painter, const QList<Data::Line> &lines) const
{
	painter->setBrush(Qt::NoBrush);

	for (int i = 0; i < lines.size(); i++) {
		const Data::Line &line = lines.at(i);
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
  const QList<TextItem*> &textItems) const
{
	QRectF rect(_rect);

	for (int i = 0; i < textItems.size(); i++) {
		const TextItem *ti = textItems.at(i);
		if (rect.intersects(ti->boundingRect()))
			ti->paint(painter);
	}
}

static QRectF lightRect(const QPointF &pos, double range)
{
	double r = qMin(range * RANGE_FACTOR, (double)TEXT_EXTENT);
	return QRect(pos.x() - r, pos.y() - r, 2 * r, 2 * r);
}

void RasterTile::drawSectorLights(QPainter *painter,
  const QMultiMap<Coordinates, SectorLight> &lights) const
{
	for (auto it = lights.cbegin(); it != lights.cend(); ++it) {
		const SectorLight &l = it.value();
		QPointF pos(ll2xy(it.key()));
		QRectF rect(lightRect(pos, (l.range == 0) ? 6 : l.range));
		double a1 = -(l.end + 90);
		double a2 = -(l.start + 90);
		if (a1 > a2)
			a2 += 360;
		double as = (a2 - a1);
		if (as == 0)
			as = 360;

		if (l.visibility == 3 || l.visibility >= 6)
			painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
		else {
			painter->setPen(QPen(Qt::black, 6,  Qt::SolidLine, Qt::FlatCap));
			painter->drawArc(rect, a1 * 16, as * 16);
			painter->setPen(QPen(Style::color(l.color), 4,  Qt::SolidLine,
			  Qt::FlatCap));
		}

		painter->drawArc(rect, a1 * 16, as * 16);

		if (a2 - a1 != 0) {
			QLineF ln(pos, QPointF(pos.x() + rect.width(), pos.y()));
			ln.setAngle(a1);
			painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
			painter->drawLine(ln);
			ln.setAngle(a2);
			painter->drawLine(ln);
		}
	}
}

void RasterTile::processPoints(const QList<Data::Point> &points,
  QList<TextItem*> &textItems, QList<TextItem*> &lightItems,
  QMultiMap<Coordinates, SectorLight> &sectorLights, bool overZoom) const
{
	QMap<Coordinates, Style::Color> lights;
	QSet<Coordinates> sigs;
	int i;

	/* Lights & Signals */
	for (i = 0; i < points.size(); i++) {
		const Data::Point &point = points.at(i);

		if (point.type()>>16 == LIGHTS) {
			const Data::Attributes &attr = point.attributes();
			Style::Color color = (Style::Color)(attr.value(COLOUR).toUInt());
			double range = attr.value(VALNMR).toDouble();

			if (attr.contains(SECTR1)
			  || (range >= MAJOR_RANGE && !(point.type() & 0xFFFF))) {
				sectorLights.insert(point.pos(), SectorLight(color,
				  attr.value(LITVIS).toUInt(), range,
				  attr.value(SECTR1).toDouble(), attr.value(SECTR2).toDouble()));
			} else
				lights.insert(point.pos(), color);
		} else if (point.type()>>16 == FOGSIG)
			sigs.insert(point.pos());
		else
			break;
	}

	/* Everything else */
	for ( ; i < points.size(); i++) {
		const Data::Point &point = points.at(i);
		QPoint pos(ll2xy(point.pos()).toPoint());
		const Style::Point &style = _style->point(point.type());

		const QString *label = point.label().isEmpty() ? 0 : &(point.label());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *fnt = (overZoom || showLabel(img, point.type()))
		  ? _style->font(style.textFontSize()) : 0;
		const QColor *color = &style.textColor();
		const QColor *hColor = style.haloColor().isValid()
		  ? &style.haloColor() : 0;
		double rotate = angle(point.type(), point.attributes().value(ORIENT));

		if ((!label || !fnt) && !img)
			continue;

		QPoint offset = img ? style.offset() : QPoint(0, 0);

		TextPointItem *item = new TextPointItem(pos + offset, label, fnt, img,
		  color, hColor, 0, 2, rotate);
		if (item->isValid() && (sectorLights.contains(point.pos())
		  || (point.polygon() && img) || !item->collides(textItems))) {
			textItems.append(item);
			if (lights.contains(point.pos()))
				lightItems.append(new TextPointItem(pos + _style->lightOffset(),
				  0, 0, _style->light(lights.value(point.pos())), 0, 0, 0, 0));
			if (sigs.contains(point.pos()))
				lightItems.append(new TextPointItem(pos + _style->signalOffset(),
				  0, 0, _style->signal(), 0, 0, 0, 0));
		} else
			delete item;
	}
}

void RasterTile::processLines(const QList<Data::Line> &lines,
  QList<TextItem*> &textItems) const
{
	for (int i = 0; i < lines.size(); i++) {
		const Data::Line &line = lines.at(i);
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

void RasterTile::drawLevels(QPainter *painter, const QList<Level> &levels)
{
	for (int i = levels.size() - 1; i >= 0; i--) {
		QList<TextItem*> textItems, lightItems;
		QMultiMap<Coordinates, SectorLight> sectorLights;
		const Level &l = levels.at(i);

		processPoints(l.points, textItems, lightItems, sectorLights, l.overZoom);
		processLines(l.lines, textItems);

		drawPolygons(painter, l.polygons);
		drawLines(painter, l.lines);
		drawArrows(painter, l.points);

		drawTextItems(painter, lightItems);
		drawSectorLights(painter, sectorLights);
		drawTextItems(painter, textItems);

		qDeleteAll(textItems);
		qDeleteAll(lightItems);
	}
}

QPainterPath RasterTile::shape(const QList<Data::Poly> &polygons) const
{
	QPainterPath shp;

	for (int i = 0; i < polygons.size(); i++) {
		const Data::Poly &p = polygons.at(i);
		if (p.type() == SUBTYPE(M_COVR, 1))
			shp.addPath(painterPath(p.path()));
	}

	return shp;
}

QList<RasterTile::Level> RasterTile::fetchLevels()
{
	QList<RasterTile::Level> list;
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

	for (int i = 0; i < _data.size(); i++) {
		Level level;

		_data.at(i)->polys(polyRectC, &level.polygons, &level.lines);
		_data.at(i)->points(pointRectC, &level.points);
		level.overZoom = i > 0;

		std::sort(level.points.begin(), level.points.end());

		if (!level.isNull())
			list.append(level);

		if (_data.size() > 1 && shape(level.polygons).contains(_rect))
			break;
	}

	return list;
}

void RasterTile::render()
{
	QList<Level> levels(fetchLevels());

	QImage img(_rect.width() * _ratio, _rect.height() * _ratio,
	  QImage::Format_ARGB32_Premultiplied);

	img.setDevicePixelRatio(_ratio);
	img.fill(Qt::transparent);

	QPainter painter(&img);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawLevels(&painter, levels);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(_rect);

	_pixmap.convertFromImage(img);
}
