#include <QFont>
#include <QPainter>
#include <QCache>
#include "map/imgmap.h"
#include "map/textpathitem.h"
#include "map/textpointitem.h"
#include "bitmapline.h"
#include "style.h"
#include "lblfile.h"
#include "rastertile.h"

using namespace IMG;

#define AREA(rect) \
	(rect.size().width() * rect.size().height())

static const QColor textColor(Qt::black);
static const QColor haloColor(Qt::white);
static const QColor shieldColor(Qt::white);
static const QColor shieldBgColor1("#dd3e3e");
static const QColor shieldBgColor2("#379947");
static const QColor shieldBgColor3("#4a7fc1");

static QFont pixelSizeFont(int pixelSize)
{
	QFont f;
	f.setPixelSize(pixelSize);
	return f;
}

static QFont *font(Style::FontSize size, Style::FontSize defaultSize
  = Style::Normal)
{
	/* The fonts must be initialized on first usage (after the QGuiApplication
	   instance is created) */
	static QFont large = pixelSizeFont(16);
	static QFont normal = pixelSizeFont(14);
	static QFont small = pixelSizeFont(12);
	static QFont extraSmall = pixelSizeFont(10);

	switch (size) {
		case Style::None:
			return 0;
		case Style::Large:
			return &large;
		case Style::Normal:
			return &normal;
		case Style::Small:
			return &small;
		case Style::ExtraSmall:
			return &extraSmall;
		default:
			return font(defaultSize);
	}
}

static QFont *poiFont(Style::FontSize size = Style::Normal)
{
	static QFont poi = pixelSizeFont(10);

	switch (size) {
		case Style::None:
			return 0;
		default:
			return &poi;
	}
}

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


void RasterTile::render()
{
	QList<TextItem*> textItems;

	ll2xy(_polygons);
	ll2xy(_lines);
	ll2xy(_points);

	processPoints(textItems);
	processPolygons(textItems);
	processLines(textItems);

	_pixmap.fill(Qt::transparent);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_xy.x(), -_xy.y());

	drawPolygons(&painter);
	drawLines(&painter);
	drawTextItems(&painter, textItems);
	//painter.setPen(Qt::red);
	//painter.drawRect(QRect(_xy, _img.size()));

	qDeleteAll(textItems);
}

void RasterTile::ll2xy(QList<MapData::Poly> &polys)
{
	for (int i = 0; i < polys.size(); i++) {
		MapData::Poly &poly = polys[i];
		for (int j = 0; j < poly.points.size(); j++) {
			QPointF &p = poly.points[j];
			p = _map->ll2xy(Coordinates(p.x(), p.y()));
		}
	}
}

void RasterTile::ll2xy(QList<MapData::Point> &points)
{
	for (int i = 0; i < points.size(); i++) {
		QPointF p(_map->ll2xy(points.at(i).coordinates));
		points[i].coordinates = Coordinates(p.x(), p.y());
	}
}

void RasterTile::drawPolygons(QPainter *painter)
{
	QCache<const LBLFile *, SubFile::Handle> hc(16);

	for (int n = 0; n < _style->drawOrder().size(); n++) {
		for (int i = 0; i < _polygons.size(); i++) {
			const MapData::Poly &poly = _polygons.at(i);
			if (poly.type != _style->drawOrder().at(n))
				continue;

			if (poly.raster.isValid()) {
				RectC r(poly.raster.rect());
				QPointF tl(_map->ll2xy(r.topLeft()));
				QPointF br(_map->ll2xy(r.bottomRight()));
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
				const Style::Polygon &style = _style->polygon(poly.type);

				painter->setPen(style.pen());
				painter->setBrush(style.brush());
				painter->drawPolygon(poly.points);
			}
		}
	}
}

void RasterTile::drawLines(QPainter *painter)
{
	painter->setBrush(Qt::NoBrush);

	for (int i = 0; i < _lines.size(); i++) {
		const MapData::Poly &poly = _lines.at(i);
		const Style::Line &style = _style->line(poly.type);

		if (style.background() == Qt::NoPen)
			continue;

		painter->setPen(style.background());
		painter->drawPolyline(poly.points);
	}

	for (int i = 0; i < _lines.size(); i++) {
		const MapData::Poly &poly = _lines.at(i);
		const Style::Line &style = _style->line(poly.type);

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

void RasterTile::processPolygons(QList<TextItem*> &textItems)
{
	QRectF tileRect(_xy, _pixmap.size());
	QSet<QString> set;
	QList<TextItem *> labels;

	for (int i = 0; i < _polygons.size(); i++) {
		MapData::Poly &poly = _polygons[i];
		bool exists = set.contains(poly.label.text());

		if (poly.label.text().isEmpty())
			continue;

		if (_zoom <= 23 && (Style::isWaterArea(poly.type)
		  || Style::isMilitaryArea(poly.type)
		  || Style::isNatureReserve(poly.type))) {
			const Style::Polygon &style = _style->polygon(poly.type);
			TextPointItem *item = new TextPointItem(
			  centroid(poly.points).toPoint(), &poly.label.text(), poiFont(),
			  0, &style.brush().color(), &haloColor);
			if (item->isValid() && !item->collides(textItems)
			  && !item->collides(labels)
			  && !(exists && tileRect.contains(item->boundingRect()))
			  && rectNearPolygon(poly.points, item->boundingRect())) {
				if (exists)
					removeDuplicitLabel(labels, poly.label.text(), tileRect);
				else
					set.insert(poly.label.text());
				labels.append(item);
			} else
				delete item;
		}
	}

	textItems.append(labels);
}

void RasterTile::processLines(QList<TextItem*> &textItems)
{
	QRect tileRect(_xy, _pixmap.size());

	std::stable_sort(_lines.begin(), _lines.end());

	if (_zoom >= 22)
		processStreetNames(tileRect, textItems);
	processShields(tileRect, textItems);
}

void RasterTile::processStreetNames(const QRect &tileRect,
  QList<TextItem*> &textItems)
{
	for (int i = 0; i < _lines.size(); i++) {
		MapData::Poly &poly = _lines[i];
		const Style::Line &style = _style->line(poly.type);

		if (style.img().isNull() && style.foreground() == Qt::NoPen)
			continue;
		if (poly.label.text().isEmpty()
		  || style.textFontSize() == Style::None)
			continue;

		const QFont *fnt = font(style.textFontSize(), Style::Small);
		const QColor *color = style.textColor().isValid()
		  ? &style.textColor() : 0;

		TextPathItem *item = new TextPathItem(poly.points,
		  &poly.label.text(), tileRect, fnt, color);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processShields(const QRect &tileRect,
  QList<TextItem*> &textItems)
{
	for (int type = FIRST_SHIELD; type <= LAST_SHIELD; type++) {
		if (minShieldZoom(static_cast<Shield::Type>(type)) > _zoom)
			continue;

		QHash<Shield, QPolygonF> shields;
		QHash<Shield, const Shield*> sp;

		for (int i = 0; i < _lines.size(); i++) {
			const MapData::Poly &poly = _lines.at(i);
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
			QRectF rect(p.boundingRect() & tileRect);
			if (AREA(rect) < AREA(QRect(0, 0, _pixmap.width()/4, _pixmap.width()/4)))
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
				  && tileRect.contains(item->boundingRect().toRect())) {
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

void RasterTile::processPoints(QList<TextItem*> &textItems)
{
	std::sort(_points.begin(), _points.end());

	for (int i = 0; i < _points.size(); i++) {
		MapData::Point &point = _points[i];
		const Style::Point &style = _style->point(point.type);
		bool poi = Style::isPOI(point.type);

		const QString *label = point.label.text().isEmpty()
		  ? 0 : &(point.label.text());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *fnt = poi
		  ? poiFont(style.textFontSize()) : font(style.textFontSize());
		const QColor *color = style.textColor().isValid()
		  ? &style.textColor() : &textColor;

		if ((!label || !fnt) && !img)
			continue;

		TextPointItem *item = new TextPointItem(QPoint(point.coordinates.lon(),
		  point.coordinates.lat()), label, fnt, img, color, &haloColor);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}
