#include <QFont>
#include <QPainter>
#include "textpathitem.h"
#include "textpointitem.h"
#include "bitmapline.h"
#include "style.h"
#include "rastertile.h"


#define AREA(rect) \
	(rect.size().width() * rect.size().height())

static const QColor shieldColor(Qt::white);
static const QColor shieldBgColor1("#dd3e3e");
static const QColor shieldBgColor2("#379947");
static const QColor shieldBgColor3("#4a7fc1");

static QString convertUnits(const QString &str)
{
	bool ok;
	int number = str.toInt(&ok);
	return ok ? QString::number(qRound(number * 0.3048)) : str;
}

static int minPOIZoom(Style::POIClass cl)
{
	switch (cl) {
		case Style::Food:
		case Style::Shopping:
		case Style::Services:
			return 26;
		case Style::Accommodation:
		case Style::Recreation:
			return 25;
		case Style::ManmadePlaces:
		case Style::NaturePlaces:
		case Style::Transport:
		case Style::Community:
		case Style::Elementary:
			return 23;
		default:
			return 0;
	}
}

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

static const QColor *shieldBgColor(Label::Shield::Type type)
{
	switch (type) {
		case Label::Shield::USInterstate:
		case Label::Shield::Hbox:
			return &shieldBgColor1;
		case Label::Shield::USShield:
		case Label::Shield::Box:
			return &shieldBgColor2;
		case Label::Shield::USRound:
		case Label::Shield::Oval:
			return &shieldBgColor3;
		default:
			return 0;
	}
}

static int minShieldZoom(Label::Shield::Type type)
{
	switch (type) {
		case Label::Shield::USInterstate:
		case Label::Shield::Hbox:
			return 17;
		case Label::Shield::USShield:
		case Label::Shield::Box:
			return 19;
		case Label::Shield::USRound:
		case Label::Shield::Oval:
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

	processPoints(textItems);
	processPolygons(textItems);
	processLines(textItems);

	_img.fill(Qt::transparent);

	QPainter painter(&_img);
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

void RasterTile::drawPolygons(QPainter *painter)
{
	for (int n = 0; n < _style->drawOrder().size(); n++) {
		for (int i = 0; i < _polygons.size(); i++) {
			const MapData::Poly &poly = _polygons.at(i);
			if (poly.type != _style->drawOrder().at(n))
				continue;
			const Style::Polygon &style = _style->polygon(poly.type);

			painter->setPen(style.pen());
			painter->setBrush(style.brush());
			painter->drawPolygon(poly.points);
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
	QRectF tileRect(_xy, _img.size());
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
			  centroid(poly.points).toPoint(), &poly.label.text(),
			  poiFont(), 0, &style.brush().color());
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
	QRect tileRect(_xy, _img.size());

	qStableSort(_lines);

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

		if (Style::isContourLine(poly.type))
			poly.label.setText(convertUnits(poly.label.text()));

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
		if (minShieldZoom(static_cast<Label::Shield::Type>(type)) > _zoom)
			continue;

		QHash<Label::Shield, QPolygonF> shields;
		QHash<Label::Shield, const Label::Shield*> sp;

		for (int i = 0; i < _lines.size(); i++) {
			const MapData::Poly &poly = _lines.at(i);
			const Label::Shield &shield = poly.label.shield();
			if (!shield.isValid() || shield.type() != type
			  || !Style::isMajorRoad(poly.type))
				continue;

			QPolygonF &p = shields[shield];
			for (int j = 0; j < poly.points.size(); j++)
				p.append(poly.points.at(j));

			sp.insert(shield, &shield);
		}

		for (QHash<Label::Shield, QPolygonF>::const_iterator it
		  = shields.constBegin(); it != shields.constEnd(); ++it) {
			const QPolygonF &p = it.value();
			QRectF rect(p.boundingRect() & tileRect);
			if (AREA(rect) < AREA(QRect(0, 0, _img.width()/4, _img.width()/4)))
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
			  poiFont(), 0, &shieldColor, shieldBgColor(it.key().type()));

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
	qSort(_points);

	for (int i = 0; i < _points.size(); i++) {
		MapData::Point &point = _points[i];
		const Style::Point &style = _style->point(point.type);
		bool poi = Style::isPOI(point.type);

		if (poi && _zoom < minPOIZoom(Style::poiClass(point.type)))
			continue;

		const QString *label = point.label.text().isEmpty()
		  ? 0 : &(point.label.text());
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *fnt = poi
		  ? poiFont(style.textFontSize()) : font(style.textFontSize());
		const QColor *color = style.textColor().isValid()
		  ? &style.textColor() : 0;

		if ((!label || !fnt) && !img)
			continue;

		if (Style::isSpot(point.type))
			point.label.setText(convertUnits(point.label.text()));
		if (Style::isSummit(point.type) && !point.label.text().isEmpty()) {
			QStringList list = point.label.text().split(" ");
			list.last() = convertUnits(list.last());
			point.label = list.join(" ");
		}

		TextPointItem *item = new TextPointItem(QPoint(point.coordinates.lon(),
		  point.coordinates.lat()), label, fnt, img, color);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

