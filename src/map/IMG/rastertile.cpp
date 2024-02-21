#include <QFont>
#include <QPainter>
#include <QCache>
#include "map/textpathitem.h"
#include "map/textpointitem.h"
#include "map/bitmapline.h"
#include "map/rectd.h"
#include "map/hillshading.h"
#include "data/dem.h"
#include "style.h"
#include "lblfile.h"
#include "rastertile.h"

using namespace IMG;

#define TEXT_EXTENT 160
#define ICON_PADDING 2

#define AREA(rect) \
	(rect.size().width() * rect.size().height())

#define HIDPI_IMG(dir, basename, ratio) \
	(((ratio) > 1.0) \
		? QImage(dir "/" basename "@2x.png") \
		: QImage(dir "/" basename ".png"))

#define ROAD  0
#define WATER 1

static const QColor textColor(Qt::black);
static const QColor haloColor(Qt::white);
static const QColor shieldColor(Qt::white);
static const QColor shieldBgColor1(0xdd, 0x3e, 0x3e);
static const QColor shieldBgColor2(0x37, 0x99, 0x47);
static const QColor shieldBgColor3(0x4a, 0x7f, 0xc1);

static const QColor *shieldBgColor(Shield::Type type)
{
	switch (type) {
		case Shield::USInterstate:
		case Shield::Hbox:
			return &shieldBgColor1;
		case Shield::USShield:
		case Shield::Box:
			return &shieldBgColor2;
		case Shield::USRound:
		case Shield::Oval:
			return &shieldBgColor3;
		default:
			return 0;
	}
}

static int minShieldZoom(Shield::Type type)
{
	switch (type) {
		case Shield::USInterstate:
		case Shield::Hbox:
			return 17;
		case Shield::USShield:
		case Shield::Box:
			return 19;
		case Shield::USRound:
		case Shield::Oval:
			return 20;
		default:
			return 0;
	}
}

static qreal area(const QVector<QPointF> &polygon)
{
	qreal area = 0;

	for (int i = 0; i < polygon.size(); i++) {
		int j = (i + 1) % polygon.size();
		area += polygon.at(i).x() * polygon.at(j).y();
		area -= polygon.at(i).y() * polygon.at(j).x();
	}
	area /= 2.0;

	return area;
}

static QPointF centroid(const QVector<QPointF> &polygon)
{
	qreal cx = 0, cy = 0;
	qreal factor = 1.0 / (6.0 * area(polygon));

	for (int i = 0; i < polygon.size(); i++) {
		int j = (i + 1) % polygon.size();
		qreal f = (polygon.at(i).x() * polygon.at(j).y() - polygon.at(j).x()
		  * polygon.at(i).y());
		cx += (polygon.at(i).x() + polygon.at(j).x()) * f;
		cy += (polygon.at(i).y() + polygon.at(j).y()) * f;
	}

	return QPointF(cx * factor, cy * factor);
}

static bool rectNearPolygon(const QPolygonF &polygon, const QRectF &rect)
{
	return (polygon.boundingRect().contains(rect)
	  && (polygon.containsPoint(rect.topLeft(), Qt::OddEvenFill)
	  || polygon.containsPoint(rect.topRight(), Qt::OddEvenFill)
	  || polygon.containsPoint(rect.bottomLeft(), Qt::OddEvenFill)
	  || polygon.containsPoint(rect.bottomRight(), Qt::OddEvenFill)));
}

const QFont *RasterTile::poiFont(Style::FontSize size, int zoom, bool extended)
{
	if (zoom > 25)
		size = Style::Normal;
	else if (extended)
		size = Style::None;

	switch (size) {
		case Style::None:
			return 0;
		default:
			return _data->style()->font(Style::ExtraSmall);
	}
}

void RasterTile::ll2xy(QList<MapData::Poly> &polys)
{
	for (int i = 0; i < polys.size(); i++) {
		MapData::Poly &poly = polys[i];
		for (int j = 0; j < poly.points.size(); j++) {
			QPointF &p = poly.points[j];
			p = ll2xy(Coordinates(p.x(), p.y()));
		}
	}
}

void RasterTile::ll2xy(QList<MapData::Point> &points)
{
	for (int i = 0; i < points.size(); i++) {
		QPointF p(ll2xy(points.at(i).coordinates));
		points[i].coordinates = Coordinates(p.x(), p.y());
	}
}

void RasterTile::drawPolygons(QPainter *painter,
  const QList<MapData::Poly> &polygons)
{
	QCache<const LBLFile *, SubFile::Handle> hc(16);

	for (int n = 0; n < _data->style()->drawOrder().size(); n++) {
		for (int i = 0; i < polygons.size(); i++) {
			const MapData::Poly &poly = polygons.at(i);
			if (poly.type != _data->style()->drawOrder().at(n))
				continue;

			if (poly.raster.isValid()) {
				RectC r(poly.raster.rect());
				QPointF tl(ll2xy(r.topLeft()));
				QPointF br(ll2xy(r.bottomRight()));
				QSizeF size(QRectF(tl, br).size());

				bool insert = false;
				SubFile::Handle *hdl = hc.object(poly.raster.lbl());
				if (!hdl) {
					hdl = new SubFile::Handle(poly.raster.lbl());
					insert = true;
				}
				QPixmap pm(poly.raster.lbl()->image(*hdl, poly.raster.id()));
				if (insert)
					hc.insert(poly.raster.lbl(), hdl);
				qreal sx = size.width() / (qreal)pm.width();
				qreal sy = size.height() / (qreal)pm.height();

				painter->save();
				painter->scale(sx, sy);
				painter->drawPixmap(QPointF(tl.x() / sx, tl.y() / sy), pm);
				painter->restore();

				//painter->setPen(Qt::blue);
				//painter->setBrush(Qt::NoBrush);
				//painter->drawRect(QRectF(tl, br));
			} else {
				const Style::Polygon &style = _data->style()->polygon(poly.type);

				painter->setPen(style.pen());
				painter->setBrush(style.brush());
				painter->drawPolygon(poly.points);
			}
		}
	}
}

void RasterTile::drawLines(QPainter *painter, const QList<MapData::Poly> &lines)
{
	painter->setBrush(Qt::NoBrush);

	for (int i = 0; i < lines.size(); i++) {
		const MapData::Poly &poly = lines.at(i);
		const Style::Line &style = _data->style()->line(poly.type);

		if (style.background() == Qt::NoPen)
			continue;

		painter->setPen(style.background());
		painter->drawPolyline(poly.points);
	}

	for (int i = 0; i < lines.size(); i++) {
		const MapData::Poly &poly = lines.at(i);
		const Style::Line &style = _data->style()->line(poly.type);

		if (!style.img().isNull())
			BitmapLine::draw(painter, poly.points, style.img());
		else if (style.foreground() != Qt::NoPen) {
			painter->setPen(style.foreground());
			painter->drawPolyline(poly.points);
		}
	}
}

void RasterTile::drawTextItems(QPainter *painter,
  const QList<TextItem*> &textItems)
{
	for (int i = 0; i < textItems.size(); i++)
		textItems.at(i)->paint(painter);
}

static void removeDuplicitLabel(QList<TextItem *> &labels, const QString &text,
  const QRectF &tileRect)
{
	for (int i = 0; i < labels.size(); i++) {
		TextItem *item = labels.at(i);
		if (tileRect.contains(item->boundingRect()) && *(item->text()) == text) {
			labels.removeAt(i);
			delete item;
			return;
		}
	}
}

void RasterTile::processPolygons(const QList<MapData::Poly> &polygons,
  QList<TextItem*> &textItems)
{
	QSet<QString> set;
	QList<TextItem *> labels;

	for (int i = 0; i < polygons.size(); i++) {
		const MapData::Poly &poly = polygons.at(i);
		bool exists = set.contains(poly.label.text());

		if (poly.label.text().isEmpty())
			continue;

		if (_zoom <= 23 && (Style::isWaterArea(poly.type)
		  || Style::isMilitaryArea(poly.type)
		  || Style::isNatureReserve(poly.type))) {
			const Style::Polygon &style = _data->style()->polygon(poly.type);
			TextPointItem *item = new TextPointItem(
			  centroid(poly.points).toPoint(), &poly.label.text(), poiFont(),
			  0, &style.brush().color(), &haloColor);
			if (item->isValid() && !item->collides(textItems)
			  && !item->collides(labels)
			  && !(exists && _rect.contains(item->boundingRect().toRect()))
			  && rectNearPolygon(poly.points, item->boundingRect())) {
				if (exists)
					removeDuplicitLabel(labels, poly.label.text(), _rect);
				else
					set.insert(poly.label.text());
				labels.append(item);
			} else
				delete item;
		}
	}

	textItems.append(labels);
}

void RasterTile::processLines(QList<MapData::Poly> &lines,
  QList<TextItem*> &textItems, const QImage (&arrows)[2])
{
	std::stable_sort(lines.begin(), lines.end());

	if (_zoom >= 22)
		processStreetNames(lines, textItems, arrows);
	processShields(lines, textItems);
}

void RasterTile::processStreetNames(const QList<MapData::Poly> &lines,
  QList<TextItem*> &textItems, const QImage (&arrows)[2])
{
	for (int i = 0; i < lines.size(); i++) {
		const MapData::Poly &poly = lines.at(i);
		const Style::Line &style = _data->style()->line(poly.type);

		if (style.img().isNull() && style.foreground() == Qt::NoPen)
			continue;

		const QFont *fnt = _data->style()->font(style.text().size(),
		  Style::Small);
		const QColor *color = style.text().color().isValid()
		  ? &style.text().color() : 0;
		const QColor *hColor = Style::isContourLine(poly.type) ? 0 : &haloColor;
		const QImage *img = poly.oneway
		  ? Style::isWaterLine(poly.type)
			? &arrows[WATER] : &arrows[ROAD] : 0;
		const QString *label = poly.label.text().isEmpty()
		  ? 0 : &poly.label.text();

		if (!img && (!label || !fnt))
			continue;

		TextPathItem *item = new TextPathItem(poly.points, label, _rect, fnt,
		  color, hColor, img);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else {
			delete item;

			if (img) {
				TextPathItem *item = new TextPathItem(poly.points, 0, _rect, 0,
				  0, 0, img);
				if (item->isValid() && !item->collides(textItems))
					textItems.append(item);
				else
					delete item;
			}
		}
	}
}

void RasterTile::processShields(const QList<MapData::Poly> &lines,
  QList<TextItem*> &textItems)
{
	for (int type = FIRST_SHIELD; type <= LAST_SHIELD; type++) {
		if (minShieldZoom(static_cast<Shield::Type>(type)) > _zoom)
			continue;

		QHash<Shield, QPolygonF> shields;
		QHash<Shield, const Shield*> sp;

		for (int i = 0; i < lines.size(); i++) {
			const MapData::Poly &poly = lines.at(i);
			const Shield &shield = poly.label.shield();
			if (!shield.isValid() || shield.type() != type
			  || !Style::isMajorRoad(poly.type))
				continue;

			QPolygonF &p = shields[shield];
			for (int j = 0; j < poly.points.size(); j++)
				p.append(poly.points.at(j));

			sp.insert(shield, &shield);
		}

		for (QHash<Shield, QPolygonF>::const_iterator it = shields.constBegin();
		  it != shields.constEnd(); ++it) {
			const QPolygonF &p = it.value();
			QRectF rect(p.boundingRect() & _rect);
			if (AREA(rect) < AREA(QRect(0, 0, _rect.width()/4, _rect.width()/4)))
				continue;

			QMap<qreal, int> map;
			QPointF center = rect.center();
			for (int j = 0; j < p.size(); j++) {
				QLineF l(p.at(j), center);
				map.insert(l.length(), j);
			}

			QMap<qreal, int>::const_iterator jt = map.constBegin();

			TextPointItem *item = new TextPointItem(
			  p.at(jt.value()).toPoint(), &(sp.value(it.key())->text()),
			  poiFont(), 0, &shieldColor, 0, shieldBgColor(it.key().type()));

			bool valid = false;
			while (true) {
				if (!item->collides(textItems)
				  && _rect.contains(item->boundingRect().toRect())) {
					valid = true;
					break;
				}
				if (++jt == map.constEnd())
					break;
				item->setPos(p.at(jt.value()).toPoint());
			}

			if (valid)
				textItems.append(item);
			else
				delete item;
		}
	}
}

void RasterTile::processPoints(QList<MapData::Point> &points,
  QList<TextItem*> &textItems)
{
	std::sort(points.begin(), points.end());

	for (int i = 0; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		const Style::Point &style = _data->style()->point(point.type);
		bool poi = Style::isPOI(point.type);

		const QString *label = point.label.text().isEmpty()
		  ? 0 : &(point.label.text());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *fnt = poi
		  ? poiFont(style.text().size(), _zoom, point.classLabel)
		  : _data->style()->font(style.text().size());
		const QColor *color = style.text().color().isValid()
		  ? &style.text().color() : &textColor;
		const QColor *hcolor = Style::isDepthPoint(point.type)
		  ? 0 : &haloColor;

		if ((!label || !fnt) && !img)
			continue;

		TextPointItem *item = new TextPointItem(QPoint(point.coordinates.lon(),
		  point.coordinates.lat()), label, fnt, img, color, hcolor, 0,
		  ICON_PADDING);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::fetchData(QList<MapData::Poly> &polygons,
  QList<MapData::Poly> &lines, QList<MapData::Point> &points)
{
	QPoint ttl(_rect.topLeft());

	QRectF polyRect(ttl, QPointF(ttl.x() + _rect.width(), ttl.y()
	  + _rect.height()));
	RectD polyRectD(_transform.img2proj(polyRect.topLeft()),
	  _transform.img2proj(polyRect.bottomRight()));
	_data->polys(polyRectD.toRectC(_proj, 20), _zoom,
	  &polygons, &lines);

	QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT, ttl.y() - TEXT_EXTENT),
	  QPointF(ttl.x() + _rect.width() + TEXT_EXTENT, ttl.y() + _rect.height()
	  + TEXT_EXTENT));
	RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
	  _transform.img2proj(pointRect.bottomRight()));
	_data->points(pointRectD.toRectC(_proj, 20), _zoom, &points);
}

Matrix RasterTile::elevation() const
{
	Matrix m(_rect.height() + 2, _rect.width() + 2);

	int left = _rect.left() - 1;
	int right = _rect.right() + 1;
	int top = _rect.top() - 1;
	int bottom = _rect.bottom() + 1;

	DEM::lock();
	for (int y = top; y <= bottom; y++) {
		for (int x = left; x <= right; x++)
			m.m(y - top, x - left) = DEM::elevation(xy2ll(QPointF(x, y)));
	}
	DEM::unlock();

	return m;
}

void RasterTile::render()
{
	QImage img(_rect.width() * _ratio, _rect.height() * _ratio,
	  QImage::Format_ARGB32_Premultiplied);
	QList<MapData::Poly> polygons;
	QList<MapData::Poly> lines;
	QList<MapData::Point> points;
	QList<TextItem*> textItems;
	QImage arrows[2];

	arrows[ROAD] = HIDPI_IMG(":/map", "arrow", _ratio);
	arrows[WATER] = HIDPI_IMG(":/map", "water-arrow", _ratio);

	fetchData(polygons, lines, points);
	ll2xy(polygons);
	ll2xy(lines);
	ll2xy(points);

	processPoints(points, textItems);
	processPolygons(polygons, textItems);
	processLines(lines, textItems, arrows);

	img.setDevicePixelRatio(_ratio);
	img.fill(Qt::transparent);

	QPainter painter(&img);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawPolygons(&painter, polygons);
	if (_hillShading && _zoom >= 18 && _zoom <= 24)
		painter.drawImage(_rect.x(), _rect.y(), HillShading::render(elevation()));
	drawLines(&painter, lines);
	drawTextItems(&painter, textItems);

	qDeleteAll(textItems);

	_pixmap = QPixmap::fromImage(img);

	//painter.setPen(Qt::red);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(_rect);
}
