#include <QFont>
#include <QPainter>
#include <QCache>
#include "common/util.h"
#include "map/dem.h"
#include "map/textpathitem.h"
#include "map/textpointitem.h"
#include "map/bitmapline.h"
#include "map/rectd.h"
#include "map/hillshading.h"
#include "map/filter.h"
#include "style.h"
#include "lblfile.h"
#include "demtree.h"
#include "rastertile.h"

using namespace IMG;

#define TEXT_EXTENT 160
#define ICON_PADDING 2

#define AREA(rect) \
	(rect.size().width() * rect.size().height())

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

static QPointF centroid(const QVector<QPointF> &polygon)
{
	qreal area = 0;
	qreal cx = 0, cy = 0;

	for (int i = 0; i < polygon.size(); i++) {
		int j = (i == polygon.size() - 1) ? 0 : i + 1;
		qreal f = (polygon.at(i).x() * polygon.at(j).y() - polygon.at(j).x()
		  * polygon.at(i).y());
		area += f;
		cx += (polygon.at(i).x() + polygon.at(j).x()) * f;
		cy += (polygon.at(i).y() + polygon.at(j).y()) * f;
	}

	qreal factor = 1.0 / (3.0 * area);

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

const QFont *RasterTile::poiFont(Style::FontSize size, int zoom,
  bool extended) const
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

void RasterTile::ll2xy(QList<MapData::Poly> &polys) const
{
	for (int i = 0; i < polys.size(); i++) {
		MapData::Poly &poly = polys[i];
		for (int j = 0; j < poly.points.size(); j++) {
			QPointF &p = poly.points[j];
			p = ll2xy(Coordinates(p.x(), p.y()));
		}
	}
}

void RasterTile::ll2xy(QList<MapData::Point> &points) const
{
	for (int i = 0; i < points.size(); i++) {
		QPointF p(ll2xy(points.at(i).coordinates));
		points[i].coordinates = Coordinates(p.x(), p.y());
	}
}

void RasterTile::drawPolygons(QPainter *painter,
  const QList<MapData::Poly> &polygons) const
{
	QCache<const LBLFile *, SubFile::Handle> hc(16);

	for (int n = 0; n < _data->style()->drawOrder().size(); n++) {
		for (int i = 0; i < polygons.size(); i++) {
			const MapData::Poly &poly = polygons.at(i);
			if (poly.type != _data->style()->drawOrder().at(n))
				continue;

			if (poly.raster.isValid()) {
				if (!_rasters)
					continue;

				RectC r(poly.raster.rect());
				QPointF tl(ll2xy(r.topLeft()));
				QPointF br(ll2xy(r.bottomRight()));
				QSizeF size(QRectF(tl, br).size());

				bool insert = false;
				SubFile::Handle *hdl = hc.object(poly.raster.lbl());
				if (!hdl) {
					hdl = new SubFile::Handle(poly.raster.lbl(), _file);
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
				//painter->setRenderHint(QPainter::Antialiasing, false);
				//painter->drawRect(QRectF(tl, br));
				//painter->setRenderHint(QPainter::Antialiasing);
			} else {
				if (!_vectors)
					continue;

				const Style::Polygon &style = _data->style()->polygon(poly.type);

				painter->setPen(style.pen());
				painter->setBrush(style.brush());
				painter->drawPolygon(poly.points);
			}
		}
	}
}

void RasterTile::drawLines(QPainter *painter,
  const QList<MapData::Poly> &lines) const
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
  const QList<TextItem*> &textItems) const
{
	QRectF rect(_rect);

	for (int i = 0; i < textItems.size(); i++) {
		const TextItem *ti = textItems.at(i);
		if (rect.intersects(ti->boundingRect()))
			ti->paint(painter);
	}
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

	if (!_vectors)
		return;

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
		  ? &style.text().color() : Style::isContourLine(poly.type)
			? 0 : &textColor;
		const QColor *hColor = Style::isContourLine(poly.type) ? 0 : &haloColor;
		const QImage *img = poly.oneway
		  ? Style::isWaterLine(poly.type)
			? &arrows[WATER] : &arrows[ROAD] : 0;
		const QString *label = poly.label.text().isEmpty()
		  ? 0 : &poly.label.text();

		if (!img && (!label || !fnt || !color))
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
		const Style *style = _data->style();
		const Style::Point &ps = style->point(point.type);
		bool poi = Style::isPOI(point.type);

		const QString *label = point.label.text().isEmpty()
		  ? 0 : &(point.label.text());
		const QImage *img = ps.img().isNull() ? 0 : &ps.img();
		const QFont *fnt = poi
		  ? poiFont(ps.text().size(), _zoom, point.flags
			& MapData::Point::ClassLabel)
		  : style->font(ps.text().size());
		const QColor *color = ps.text().color().isValid()
		  ? &ps.text().color() : &textColor;
		const QColor *hcolor = Style::isDepthPoint(point.type)
		  ? 0 : &haloColor;

		if ((!label || !fnt) && !img)
			continue;

		QPoint pos(point.coordinates.lon(), point.coordinates.lat());
		QPoint offset = img ? ps.offset() : QPoint(0, 0);

		TextPointItem *item = new TextPointItem(pos + offset, label, fnt, img,
		  color, hcolor, 0, ICON_PADDING);
		if (item->isValid() && !item->collides(textItems)) {
			textItems.append(item);
			if (Style::isLight(point.type) || point.flags & MapData::Point::Light)
				textItems.append(new TextPointItem(pos + style->lightOffset(),
				  0, 0, style->light(), 0, 0, 0, 0));
		} else
			delete item;
	}
}

void RasterTile::fetchData(QList<MapData::Poly> &polygons,
  QList<MapData::Poly> &lines, QList<MapData::Point> &points)
{
	QPoint ttl(_rect.topLeft());

	if (dynamic_cast<IMGData*>(_data)) {
		_file = new QFile(_data->fileName());
		if (!_file->open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
			qWarning("%s: %s", qPrintable(_file->fileName()),
			  qPrintable(_file->errorString()));
			return;
		}
	}

	QRectF polyRect(ttl, QPointF(ttl.x() + _rect.width(), ttl.y()
	  + _rect.height()));
	RectD polyRectD(_transform.img2proj(polyRect.topLeft()),
	  _transform.img2proj(polyRect.bottomRight()));
	_data->polys(_file, polyRectD.toRectC(_proj, 20), _zoom,
	  &polygons, _vectors ? &lines : 0);

	if (_vectors) {
		QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT, ttl.y() - TEXT_EXTENT),
		  QPointF(ttl.x() + _rect.width() + TEXT_EXTENT, ttl.y() + _rect.height()
		  + TEXT_EXTENT));
		RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
		  _transform.img2proj(pointRect.bottomRight()));
		_data->points(_file, pointRectD.toRectC(_proj, 20), _zoom, &points);
	}
}

MatrixD RasterTile::elevation(int extend) const
{
	int left = _rect.left() - extend;
	int right = _rect.right() + extend;
	int top = _rect.top() - extend;
	int bottom = _rect.bottom() + extend;

	MatrixC ll(_rect.height() + 2 * extend, _rect.width() + 2 * extend);
	for (int y = top, i = 0; y <= bottom; y++)
		for (int x = left; x <= right; x++, i++)
			ll.at(i) = xy2ll(QPointF(x, y));

	if (_data->hasDEM()) {
		RectC rect;
		QList<MapData::Elevation> tiles;

		for (int i = 0; i < ll.size(); i++)
			rect = rect.united(ll.at(i));
		// Extra margin for always including the next DEM tile on the map tile
		// edges (the DEM tile resolution is usally 0.5-15% of the map tile)
		double factor = 6 - (_zoom - 24) * 1.7;
		_data->elevations(_file, rect.adjusted(0, 0, rect.width() / factor,
		  -rect.height() / factor), _zoom, &tiles);

		DEMTree tree(tiles);
		MatrixD m(ll.h(), ll.w());
		for (int i = 0; i < ll.size(); i++)
			m.at(i) = tree.elevation(ll.at(i));

		return m;
	} else
		return DEM::elevation(ll);
}

void RasterTile::drawHillShading(QPainter *painter) const
{
	if (_hillShading && _zoom >= 18 && _zoom <= 24) {
		if (HillShading::blur()) {
			MatrixD dem(Filter::blur(elevation(HillShading::blur() + 1),
			  HillShading::blur()));
			QImage img(HillShading::render(dem, HillShading::blur() + 1));
			painter->drawImage(_rect.x(), _rect.y(), img);
		} else
			painter->drawImage(_rect.x(), _rect.y(),
			  HillShading::render(elevation(1), 1));
	}
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

	arrows[ROAD] = Util::svg2img(":/symbols/oneway.svg", _ratio);
	arrows[WATER] = Util::svg2img(":/symbols/flow.svg", _ratio);

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
	drawHillShading(&painter);
	drawLines(&painter, lines);
	drawTextItems(&painter, textItems);

	qDeleteAll(textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(_rect);

	_pixmap.convertFromImage(img);
}
