#include <QPainter>
#include "map/bitmapline.h"
#include "map/textpointitem.h"
#include "map/textpathitem.h"
#include "style.h"
#include "rastertile.h"

using namespace ENC;

#define ICON_PADDING 2

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

void RasterTile::drawPolygons(QPainter *painter)
{
	const Style &s = style();

	for (int n = 0; n < s.drawOrder().size(); n++) {
		for (int i = 0; i < _polygons.size(); i++) {
			const MapData::Poly *poly = _polygons.at(i);
			if (poly->type() != s.drawOrder().at(n))
				continue;
			const Style::Polygon &style = s.polygon(poly->type());

			painter->setPen(style.pen());
			painter->setBrush(style.brush());
			painter->drawPath(painterPath(poly->path()));
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

		if ((!label || !fnt) && !img)
			continue;

		TextPointItem *item = new TextPointItem(ll2xy(point->pos()).toPoint(),
		  label, fnt, img, color, &haloColor, 0, ICON_PADDING);
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

	processPoints(textItems);
	processLines(textItems);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawPolygons(&painter);
	drawLines(&painter);

	drawTextItems(&painter, textItems);

	qDeleteAll(textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(QRect(_rect.topLeft(), _pixmap.size()));
}
