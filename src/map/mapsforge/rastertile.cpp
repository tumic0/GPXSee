#include <cmath>
#include <QPainter>
#include <QCache>
#include "common/programpaths.h"
#include "map/rectd.h"
#include "rastertile.h"

using namespace Mapsforge;

#define TEXT_EXTENT 160

static double limit = cos(deg2rad(170));

static qreal area(const QPainterPath &polygon)
{
	qreal area = 0;

	for (int i = 0; i < polygon.elementCount(); i++) {
		int j = (i + 1) % polygon.elementCount();
		area += polygon.elementAt(i).x * polygon.elementAt(j).y;
		area -= polygon.elementAt(i).y * polygon.elementAt(j).x;
	}
	area /= 2.0;

	return area;
}

static QPointF centroid(const QPainterPath &polygon)
{
	qreal cx = 0, cy = 0;
	qreal factor = 1.0 / (6.0 * area(polygon));

	for (int i = 0; i < polygon.elementCount(); i++) {
		int j = (i + 1) % polygon.elementCount();
		qreal f = (polygon.elementAt(i).x * polygon.elementAt(j).y
		  - polygon.elementAt(j).x * polygon.elementAt(i).y);
		cx += (polygon.elementAt(i).x + polygon.elementAt(j).x) * f;
		cy += (polygon.elementAt(i).y + polygon.elementAt(j).y) * f;
	}

	return QPointF(cx * factor, cy * factor);
}

static const QByteArray *label(unsigned key, const QVector<MapData::Tag> &tags)
{
	for (int i = 0; i < tags.size(); i++) {
		const MapData::Tag &tag = tags.at(i);
		if (tag.key == key)
			return tag.value.isEmpty() ? 0 : &tag.value;
	}

	return 0;
}

static const QColor *haloColor(const Style::TextRender *ti)
{
	return (ti->strokeColor() != ti->fillColor() && ti->strokeWidth() > 0)
	  ? &ti->strokeColor() : 0;
}

static QPainterPath parallelPath(const QPainterPath &p, double dy)
{
	int n = p.elementCount() - 1;
	QVector<QPointF> u(n);
	QPainterPath h;

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
	h.reserve(p.elementCount());
#endif // QT 5.13

	for (int k = 0; k < n; k++) {
		qreal c = p.elementAt(k + 1).x - p.elementAt(k).x;
		qreal s = p.elementAt(k + 1).y - p.elementAt(k).y;
		qreal l = sqrt(c * c + s * s);

		u[k] = (l == 0) ? QPointF(0, 0) : QPointF(c / l, s / l);

		if (k == 0)
			continue;
		if (u.at(k).x() * u.at(k-1).x() + u.at(k).y() * u.at(k-1).y() < limit)
			return p;
	}

	h.moveTo(QPointF(p.elementAt(0).x - dy * u.at(0).y(),
	  p.elementAt(0).y + dy * u.at(0).x()));

	for (int k = 1; k < n; k++) {
		qreal l = dy / (1 + u.at(k).x() * u.at(k-1).x()
		  + u.at(k).y() * u.at(k-1).y());
		QPainterPath::Element e(p.elementAt(k));

		h.lineTo(QPointF(e.x - l * (u.at(k).y() + u.at(k-1).y()),
		  e.y + l * (u.at(k).x() + u.at(k-1).x())));
	}

	h.lineTo(QPointF(p.elementAt(n).x - dy * u.at(n-1).y(),
	  p.elementAt(n).y + dy * u.at(n-1).x()));

	return h;
}

void RasterTile::processPointLabels(const QList<MapData::Point> &points,
  QList<TextItem*> &textItems)
{
	QList<const Style::TextRender*> labels(_style->pointLabels(_zoom));
	QList<const Style::Symbol*> symbols(_style->pointSymbols(_zoom));
	QList<PainterPoint> painterPoints;

	for (int i = 0; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		const QByteArray *lbl = 0;
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(point.tags)) {
				if ((lbl = label(ri->key(), point.tags))) {
					ti = ri;
					break;
				}
			}
		}

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);
			if (ri->rule().match(point.tags)) {
				si = ri;
				break;
			}
		}

		if (ti || si)
			painterPoints.append(PainterPoint(&point, lbl, si, ti));
	}

	std::sort(painterPoints.begin(), painterPoints.end());

	for (int i = 0; i < painterPoints.size(); i++) {
		const PainterPoint &p = painterPoints.at(i);
		const QImage *img = p.si ? &p.si->img() : 0;
		const QFont *font = p.ti ? &p.ti->font() : 0;
		const QColor *color = p.ti ? &p.ti->fillColor() : 0;
		const QColor *hColor = p.ti ? haloColor(p.ti) : 0;

		PointItem *item = new PointItem(ll2xy(p.p->coordinates).toPoint(),
		  p.lbl, font, img, color, hColor);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processAreaLabels(QList<TextItem*> &textItems,
  QVector<PainterPath> &paths)
{
	QList<const Style::TextRender*> labels(_style->areaLabels(_zoom));
	QList<const Style::Symbol*> symbols(_style->areaSymbols(_zoom));

	for (int i = 0; i < paths.size(); i++) {
		PainterPath &path = paths[i];
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;
		const QByteArray *lbl = 0;

		if (!path.path->closed)
			continue;

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(path.path->closed, path.path->tags)) {
				if ((lbl = label(ri->key(), path.path->tags)))
					ti = ri;
				break;
			}
		}

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);
			if (ri->rule().match(path.path->tags)) {
				si = ri;
				break;
			}
		}

		if (!ti && !si)
			continue;

		const QImage *img = si ? &si->img() : 0;
		const QFont *font = ti ? &ti->font() : 0;
		const QColor *color = ti ? &ti->fillColor() : 0;
		const QColor *hColor = ti ? haloColor(ti) : 0;
		QPointF pos = path.path->labelPos.isNull()
		  ? centroid(path.pp) : ll2xy(path.path->labelPos);

		PointItem *item = new PointItem(pos.toPoint(), lbl, font, img, color,
		  hColor);
		if (item->isValid() && _rect.contains(item->boundingRect().toRect())
		  && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processLineLabels(QList<TextItem*> &textItems,
  QVector<PainterPath> &paths)
{
	QList<const Style::TextRender*> instructions(_style->pathLabels(_zoom));
	QSet<QByteArray> set;

	for (int i = 0; i < instructions.size(); i++) {
		const Style::TextRender *ri = instructions.at(i);

		for (int i = 0; i < paths.size(); i++) {
			PainterPath &path = paths[i];
			const QByteArray *lbl = label(ri->key(), path.path->tags);

			if (!lbl)
				continue;
			if (!ri->rule().match(path.path->closed, path.path->tags))
				continue;
			bool limit = (ri->key() == ID_ELE || ri->key() == ID_REF);
			if (limit && set.contains(*lbl))
				continue;

			PathItem *item = new PathItem(path.pp, lbl, _rect, &ri->font(),
			  &ri->fillColor(), haloColor(ri));
			if (item->isValid() && !item->collides(textItems)) {
				textItems.append(item);
				if (limit)
					set.insert(*lbl);
			} else
				delete item;
		}
	}
}

void RasterTile::drawTextItems(QPainter *painter,
  const QList<TextItem*> &textItems)
{
	for (int i = 0; i < textItems.size(); i++)
		textItems.at(i)->paint(painter);
}

QPainterPath RasterTile::painterPath(const Polygon &polygon, bool curve) const
{
	QPainterPath path;

	if (curve) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
		int size = 0;
		for (int i = 0; i < polygon.size(); i++)
			size += polygon.at(i).size();
		path.reserve(size);
#endif // QT 5.13

		for (int i = 0; i < polygon.size(); i++) {
			const QVector<Coordinates> &subpath = polygon.at(i);

			QPointF p1(ll2xy(subpath.first()));
			QPointF p2(0, 0);
			QPointF p3(0, 0);

			path.moveTo(p1);
			for (int j = 1; j < subpath.size(); j++) {
				p3 = ll2xy(subpath.at(j));
				p2 = QPointF((p1.x() + p3.x()) / 2.0, (p1.y() + p3.y()) / 2.0);
				path.quadTo(p1, p2);
				p1 = p3;
			}
			path.quadTo(p2, p3);
		}
	} else {
		for (int i = 0; i < polygon.size(); i++) {
			const QVector<Coordinates> &subpath = polygon.at(i);

			QVector<QPointF> p(subpath.size());
			for (int j = 0; j < subpath.size(); j++)
				p[j] = ll2xy(subpath.at(j));
			path.addPolygon(p);
		}
	}

	return path;
}

void RasterTile::pathInstructions(const QList<MapData::Path> &paths,
  QVector<PainterPath> &painterPaths,
  QVector<RasterTile::RenderInstruction> &instructions)
{
	QCache<PathKey, QList<const Style::PathRender *> > cache(8192);
	QList<const Style::PathRender*> *ri;

	for (int i = 0; i < paths.size(); i++) {
		const MapData::Path &path = paths.at(i);
		PainterPath &rp = painterPaths[i];
		PathKey key(_zoom, path.closed, path.tags);

		rp.path = &path;

		if (!(ri = cache.object(key))) {
			ri = new QList<const Style::PathRender*>(_style->paths(_zoom,
			  path.closed, path.tags));
			for (int j = 0; j < ri->size(); j++)
				instructions.append(RenderInstruction(ri->at(j), &rp));
			cache.insert(key, ri);
		} else {
			for (int j = 0; j < ri->size(); j++)
				instructions.append(RenderInstruction(ri->at(j), &rp));
		}
	}
}

void RasterTile::circleInstructions(const QList<MapData::Point> &points,
  QVector<RasterTile::RenderInstruction> &instructions)
{
	QCache<PointKey, QList<const Style::CircleRender *> > cache(8192);
	QList<const Style::CircleRender*> *ri;

	for (int i = 0; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		PointKey key(_zoom, point.tags);

		if (!(ri = cache.object(key))) {
			ri = new QList<const Style::CircleRender*>(_style->circles(_zoom,
			  point.tags));
			for (int j = 0; j < ri->size(); j++)
				instructions.append(RenderInstruction(ri->at(j), &point));
			cache.insert(key, ri);
		} else {
			for (int j = 0; j < ri->size(); j++)
				instructions.append(RenderInstruction(ri->at(j), &point));
		}
	}
}

void RasterTile::drawPaths(QPainter *painter, const QList<MapData::Path> &paths,
  const QList<MapData::Point> &points, QVector<PainterPath> &painterPaths)
{
	QVector<RenderInstruction> instructions;
	pathInstructions(paths, painterPaths, instructions);
	circleInstructions(points, instructions);
	std::sort(instructions.begin(), instructions.end());

	for (int i = 0; i < instructions.size(); i++) {
		const RenderInstruction &is = instructions.at(i);
		PainterPath *path = is.path();

		if (path) {
			const Style::PathRender *ri = is.pathRender();
			qreal dy = ri->dy(_zoom);

			if (!path->pp.elementCount())
				path->pp = painterPath(path->path->poly, ri->curve());

			painter->setPen(ri->pen(_zoom));
			painter->setBrush(ri->brush());
			if (dy != 0)
				painter->drawPath(parallelPath(path->pp, dy));
			else
				painter->drawPath(path->pp);
		} else {
			const Style::CircleRender *ri = is.circleRender();
			qreal radius = ri->radius(_zoom);

			painter->setPen(ri->pen());
			painter->setBrush(ri->brush());
			painter->drawEllipse(ll2xy(is.point()->coordinates), radius, radius);
		}
	}
}

void RasterTile::fetchData(QList<MapData::Path> &paths,
  QList<MapData::Point> &points)
{
	QPoint ttl(_rect.topLeft());

	/* Add a "sub-pixel" margin to assure the tile areas do not
	   overlap on the border lines. This prevents areas overlap
	   artifacts at least when using the EPSG:3857 projection. */
	QRectF pathRect(QPointF(ttl.x() + 0.5, ttl.y() + 0.5),
	  QPointF(ttl.x() + _rect.width() - 0.5, ttl.y() + _rect.height() - 0.5));
	RectD pathRectD(_transform.img2proj(pathRect.topLeft()),
	  _transform.img2proj(pathRect.bottomRight()));
	_data->paths(pathRectD.toRectC(_proj, 20), _zoom, &paths);

	QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT, ttl.y() - TEXT_EXTENT),
	  QPointF(ttl.x() + _rect.width() + TEXT_EXTENT, ttl.y() + _rect.height()
	  + TEXT_EXTENT));
	RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
	  _transform.img2proj(pointRect.bottomRight()));
	_data->points(pointRectD.toRectC(_proj, 20), _zoom, &points);
}

void RasterTile::render()
{
	QList<MapData::Path> paths;
	QList<MapData::Point> points;

	fetchData(paths, points);

	QList<TextItem*> textItems;
	QVector<PainterPath> renderPaths(paths.size());

	_pixmap.setDevicePixelRatio(_ratio);
	_pixmap.fill(Qt::transparent);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawPaths(&painter, paths, points, renderPaths);

	processPointLabels(points, textItems);
	processAreaLabels(textItems, renderPaths);
	processLineLabels(textItems, renderPaths);
	drawTextItems(&painter, textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(QRect(_rect.topLeft(), _pixmap.size()));

	qDeleteAll(textItems);

	_valid = true;
}
