#include <QtMath>
#include <QPainter>
#include "common/linec.h"
#include "map/bitmapline.h"
#include "map/textpointitem.h"
#include "map/textpathitem.h"
#include "style.h"
#include "rastertile.h"

using namespace ENC;

#define ICON_PADDING 2
#define ARROW_SIZE 0.005

const float C1 = 0.866025f; /* sqrt(3)/2 */

static const QColor haloColor(Qt::white);

static struct {
	bool operator()(MapData::Point* a, MapData::Point* b) const
	  {return *a < *b;}
} pointLess;

static QFont pixelSizeFont(int pixelSize)
{
	QFont f;
	f.setPixelSize(pixelSize);
	return f;
}

static QFont *font(Style::FontSize size)
{
	/* The fonts must be initialized on first usage (after the QGuiApplication
	   instance is created) */
	static QFont large = pixelSizeFont(16);
	static QFont normal = pixelSizeFont(12);
	static QFont small = pixelSizeFont(10);

	switch (size) {
		case Style::None:
			return 0;
		case Style::Large:
			return &large;
		case Style::Small:
			return &small;
		default:
			return &normal;
	}
}

static const Style& style()
{
	static Style s;
	return s;
}

static double area(const QVector<Coordinates> &polygon)
{
	double area = 0;

	for (int i = 0; i < polygon.size(); i++) {
		int j = (i + 1) % polygon.size();
		area += polygon.at(i).lon() * polygon.at(j).lat();
		area -= polygon.at(i).lat() * polygon.at(j).lon();
	}
	area /= 2.0;

	return area;
}

static Coordinates centroid(const QVector<Coordinates> &polygon)
{
	double cx = 0, cy = 0;
	double factor = 1.0 / (6.0 * area(polygon));

	for (int i = 0; i < polygon.size(); i++) {
		int j = (i + 1) % polygon.size();
		qreal f = (polygon.at(i).lon() * polygon.at(j).lat()
		  - polygon.at(j).lon() * polygon.at(i).lat());
		cx += (polygon.at(i).lon() + polygon.at(j).lon()) * f;
		cy += (polygon.at(i).lat() + polygon.at(j).lat()) * f;
	}

	return Coordinates(cx * factor, cy * factor);
}

QPainterPath RasterTile::painterPath(const Polygon &polygon) const
{
	QPainterPath path;

	for (int i = 0; i < polygon.size(); i++) {
		const QVector<Coordinates> &subpath = polygon.at(i);

		path.moveTo(ll2xy(subpath.first()));
		for (int j = 1; j < subpath.size(); j++)
			path.lineTo(ll2xy(subpath.at(j)));
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

QPolygonF RasterTile::arrow(const Coordinates &c, qreal angle) const
{
	Coordinates t[3], r[4];
	QPolygonF polygon;

	t[0] = c;
	t[1] = Coordinates(t[0].lon() - qCos(angle - M_PI/3) * ARROW_SIZE,
	  t[0].lat() - qSin(angle - M_PI/3) * ARROW_SIZE);
	t[2] = Coordinates(t[0].lon() - qCos(angle - M_PI + M_PI/3) * ARROW_SIZE,
	  t[0].lat() - qSin(angle - M_PI + M_PI/3) * ARROW_SIZE);

	LineC l(t[1], t[2]);
	r[0] = l.pointAt(0.25);
	r[1] = l.pointAt(0.75);
	r[2] = Coordinates(r[0].lon() - C1 * ARROW_SIZE * qCos(angle - M_PI/2),
	  r[0].lat() - C1 * ARROW_SIZE * qSin(angle - M_PI/2));
	r[3] = Coordinates(r[1].lon() - C1 * ARROW_SIZE * qCos(angle - M_PI/2),
	  r[1].lat() - C1 * ARROW_SIZE * qSin(angle - M_PI/2));

	polygon << ll2xy(t[0]) << ll2xy(t[2]) << ll2xy(r[1]) << ll2xy(r[3])
	  << ll2xy(r[2]) << ll2xy(r[0]) << ll2xy(t[1]);

	return polygon;
}

void RasterTile::drawArrows(QPainter *painter)
{
	painter->setPen(QPen(QColor("#eb49eb"), 1));
	painter->setBrush(QBrush("#80eb49eb"));

	for (int i = 0; i < _polygons.size(); i++) {
		const MapData::Poly *poly = _polygons.at(i);

		if (poly->type()>>16 == TSSLPT) {
			qreal angle = (poly->type() & 0xFFFF) / 10.0;
			QPolygonF polygon(arrow(centroid(poly->path().first()),
			  deg2rad(180 - angle)));
			painter->drawPolygon(polygon);
		}
	}
}

void RasterTile::drawPolygons(QPainter *painter)
{
	const Style &s = style();

	for (int n = 0; n < s.drawOrder().size(); n++) {
		for (int i = 0; i < _polygons.size(); i++) {
			const MapData::Poly *poly = _polygons.at(i);
			if (poly->type() != s.drawOrder().at(n))
				continue;
			const Style::Polygon &style = s.polygon(poly->type());

			if (!style.img().isNull()) {
				for (int i = 0; i < poly->path().size(); i++)
					BitmapLine::draw(painter, polyline(poly->path().at(i)),
					  style.img());
			} else {
				painter->setPen(style.pen());
				painter->setBrush(style.brush());
				painter->drawPath(painterPath(poly->path()));
			}
		}
	}
}

void RasterTile::drawLines(QPainter *painter)
{
	const Style &s = style();

	painter->setBrush(Qt::NoBrush);

	for (int i = 0; i < _lines.size(); i++) {
		const MapData::Line *line = _lines.at(i);
		const Style::Line &style = s.line(line->type());

		if (!style.img().isNull()) {
			BitmapLine::draw(painter, polyline(line->path()), style.img());
		} else if (style.pen() != Qt::NoPen) {
			painter->setPen(style.pen());
			painter->drawPolyline(polyline(line->path()));
		}
	}
}

void RasterTile::drawTextItems(QPainter *painter,
  const QList<TextItem*> &textItems)
{
	for (int i = 0; i < textItems.size(); i++)
		textItems.at(i)->paint(painter);
}

void RasterTile::processPolygons(QList<TextItem*> &textItems)
{
	const Style &s = style();

	for (int i = 0; i < _polygons.size(); i++) {
		const MapData::Poly *poly = _polygons.at(i);
		uint type = poly->type()>>16;

		if (!(type == HRBFAC || type == I_TRNBSN))
			continue;
		const Style::Point &style = s.point(poly->type());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		if (!img)
			continue;

		TextPointItem *item = new TextPointItem(
		  ll2xy(centroid(poly->path().first())).toPoint(),
		  0, 0, img, 0, 0, 0, 0);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processPoints(QList<TextItem*> &textItems)
{
	const Style &s = style();

	std::sort(_points.begin(), _points.end(), pointLess);

	for (int i = 0; i < _points.size(); i++) {
		const MapData::Point *point = _points.at(i);
		const Style::Point &style = s.point(point->type());

		const QString *label = point->label().isEmpty() ? 0 : &(point->label());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *fnt = font(style.textFontSize());
		const QColor *color = &style.textColor();
		const QColor *hColor = (Style::isSounding(point->type())
		  || Style::isDistanceMark(point->type())) ? 0 : &haloColor;

		if ((!label || !fnt) && !img)
			continue;

		TextPointItem *item = new TextPointItem(ll2xy(point->pos()).toPoint(),
		  label, fnt, img, color, hColor, 0, ICON_PADDING);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processLines(QList<TextItem*> &textItems)
{
	const Style &s = style();

	for (int i = 0; i < _lines.size(); i++) {
		const MapData::Line *line = _lines.at(i);
		const Style::Line &style = s.line(line->type());

		if (style.img().isNull() && style.pen() == Qt::NoPen)
			continue;
		if (line->label().isEmpty() || style.textFontSize() == Style::None)
			continue;

		const QFont *fnt = font(style.textFontSize());
		const QColor *color = &style.textColor();

		TextPathItem *item = new TextPathItem(polyline(line->path()),
		  &line->label(), _rect, fnt, color, 0);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::render()
{
	QList<TextItem*> textItems;

	_pixmap.setDevicePixelRatio(_ratio);
	_pixmap.fill(Qt::transparent);

	processPolygons(textItems);
	processPoints(textItems);
	processLines(textItems);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawPolygons(&painter);
	drawLines(&painter);
	drawArrows(&painter);

	drawTextItems(&painter, textItems);

	qDeleteAll(textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(QRect(_rect.topLeft(), _pixmap.size()));

	_valid = true;
}
