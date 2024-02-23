#include <cmath>
#include <QPainter>
#include <QCache>
#include "common/dem.h"
#include "map/rectd.h"
#include "map/hillshading.h"
#include "rastertile.h"

using namespace Mapsforge;

#define TEXT_EXTENT 160
#define PATHS_EXTENT 20
#define SEARCH_EXTENT -0.5

static double LIMIT = cos(deg2rad(170));

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
		if (u.at(k).x() * u.at(k-1).x() + u.at(k).y() * u.at(k-1).y() < LIMIT)
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
  QList<TextItem*> &textItems) const
{
	QList<const Style::TextRender*> labels(_style->pointLabels(_zoom));
	QList<const Style::Symbol*> symbols(_style->pointSymbols(_zoom));
	QList<PointText> items;

	for (int i = 0; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;
		const QByteArray *lbl = 0;

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);

			if (ri->rule().match(point.tags))
				if (!si || si->priority() < ri->priority())
					si = ri;
		}

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(point.tags)) {
				if ((lbl = label(ri->key(), point.tags))) {
					if (si && si->id() != ri->symbolId())
						continue;
					if (!ti || ti->priority() < ri->priority())
						ti = ri;
				}
			}
		}

		if (ti || si)
			items.append(PointText(&point, lbl, si, ti));
	}

	std::sort(items.begin(), items.end());

	for (int i = 0; i < items.size(); i++) {
		const PointText &p = items.at(i);
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

void RasterTile::processAreaLabels(const QVector<PainterPath> &paths,
  QList<TextItem*> &textItems) const
{
	QList<const Style::TextRender*> labels(_style->areaLabels(_zoom));
	QList<const Style::Symbol*> symbols(_style->areaSymbols(_zoom));
	QList<PathText> items;

	for (int i = 0; i < paths.size(); i++) {
		const PainterPath &path = paths.at(i);
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;
		const QByteArray *lbl = 0;

		if (!path.path->closed)
			continue;

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);

			if (ri->rule().match(path.path->closed, path.path->tags))
				if (!si || si->priority() < ri->priority())
					si = ri;
		}

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(path.path->closed, path.path->tags)) {
				if ((lbl = label(ri->key(), path.path->tags))) {
					if (si && si->id() != ri->symbolId())
						continue;

					ti = ri;
					break;
				}
			}
		}

		if (ti || si)
			items.append(PathText(&path, lbl, si, ti));
	}

	std::sort(items.begin(), items.end());

	for (int i = 0; i < items.size(); i++) {
		const PathText &p = items.at(i);
		const QImage *img = p.si ? &p.si->img() : 0;
		const QFont *font = p.ti ? &p.ti->font() : 0;
		const QColor *color = p.ti ? &p.ti->fillColor() : 0;
		const QColor *hColor = p.ti ? haloColor(p.ti) : 0;
		QPointF pos = p.p->path->labelPos.isNull()
		  ? centroid(p.p->pp) : ll2xy(p.p->path->labelPos);

		PointItem *item = new PointItem(pos.toPoint(), p.lbl, font, img, color,
		  hColor);
		if (item->isValid() && _rect.contains(item->boundingRect().toRect())
		  && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processLineLabels(const QVector<PainterPath> &paths,
  QList<TextItem*> &textItems) const
{
	QList<const Style::TextRender*> labels(_style->pathLabels(_zoom));
	QList<const Style::Symbol*> symbols(_style->lineSymbols(_zoom));
	QList<PathText> items;
	QSet<QByteArray> set;

	for (int i = 0; i < paths.size(); i++) {
		const PainterPath &path = paths.at(i);
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;
		const QByteArray *lbl = 0;

		if (path.path->closed)
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
			if (ri->rule().match(path.path->closed, path.path->tags)) {
				si = ri;
				break;
			}
		}

		if (ti || si)
			items.append(PathText(&path, lbl, si, ti));
	}

	std::sort(items.begin(), items.end());

	for (int i = 0; i < items.size(); i++) {
		const PathText &p = items.at(i);
		const QImage *img = p.si ? &p.si->img() : 0;
		const QFont *font = p.ti ? &p.ti->font() : 0;
		const QColor *color = p.ti ? &p.ti->fillColor() : 0;
		const QColor *hColor = p.ti ? haloColor(p.ti) : 0;
		bool rotate = p.si ? p.si->rotate() : false;
		bool limit = false;

		if (p.ti) {
			limit = (p.ti->key() == ID_ELE || p.ti->key() == ID_REF);
			if (limit && set.contains(*p.lbl))
				continue;
		}

		PathItem *item = new PathItem(p.p->pp, p.lbl, img, _rect, font, color,
		  hColor, rotate);
		if (item->isValid() && !item->collides(textItems)) {
			textItems.append(item);
			if (limit)
				set.insert(*p.lbl);
		} else {
			delete item;

			if (img && p.lbl) {
				PathItem *item = new PathItem(p.p->pp, 0, img, _rect, 0, 0, 0,
				  rotate);
				if (item->isValid() && !item->collides(textItems))
					textItems.append(item);
				else
					delete item;
			}
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
  QVector<RasterTile::RenderInstruction> &instructions) const
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
  QVector<RasterTile::RenderInstruction> &instructions) const
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

void RasterTile::hillShadingInstructions(
  QVector<RasterTile::RenderInstruction> &instructions) const
{
	const Style::HillShadingRender *hs = _style->hillShading(_zoom);
	if (hs)
		instructions.append(RenderInstruction(hs));
}

void RasterTile::drawPaths(QPainter *painter, const QList<MapData::Path> &paths,
  const QList<MapData::Point> &points, QVector<PainterPath> &painterPaths)
{
	QVector<RenderInstruction> instructions;
	pathInstructions(paths, painterPaths, instructions);
	circleInstructions(points, instructions);
	hillShadingInstructions(instructions);
	std::sort(instructions.begin(), instructions.end());

	for (int i = 0; i < instructions.size(); i++) {
		const RenderInstruction &is = instructions.at(i);
		PainterPath *path = is.path();
		const MapData::Point *point = is.point();

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
		} else if (point) {
			const Style::CircleRender *ri = is.circleRender();
			qreal radius = ri->radius(_zoom);

			painter->setPen(ri->pen());
			painter->setBrush(ri->brush());
			painter->drawEllipse(ll2xy(point->coordinates), radius, radius);
		} else {
			if (_hillShading)
				painter->drawImage(_rect.x(), _rect.y(),
				  HillShading::render(elevation()));
		}
	}
}

void RasterTile::fetchData(QList<MapData::Path> &paths,
  QList<MapData::Point> &points) const
{
	QPoint ttl(_rect.topLeft());

	QRectF pathRect(QPointF(ttl.x() - PATHS_EXTENT, ttl.y() - PATHS_EXTENT),
	  QPointF(ttl.x() + _rect.width() + PATHS_EXTENT, ttl.y() + _rect.height()
	  + PATHS_EXTENT));
	QRectF searchRect(QPointF(ttl.x() - SEARCH_EXTENT, ttl.y() - SEARCH_EXTENT),
	  QPointF(ttl.x() + _rect.width() + SEARCH_EXTENT, ttl.y() + _rect.height()
	  + SEARCH_EXTENT));
	RectD pathRectD(_transform.img2proj(pathRect.topLeft()),
	  _transform.img2proj(pathRect.bottomRight()));
	RectD searchRectD(_transform.img2proj(searchRect.topLeft()),
	  _transform.img2proj(searchRect.bottomRight()));
	_data->paths(searchRectD.toRectC(_proj, 20), pathRectD.toRectC(_proj, 20),
	  _zoom, &paths);

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

	QVector<Coordinates> ll;
	ll.reserve(m.w() * m.h());
	for (int y = top; y <= bottom; y++) {
		for (int x = left; x <= right; x++)
			ll.append(xy2ll(QPointF(x, y)));
	}

	DEM::lock();
	for (int i = 0; i < ll.size(); i++)
		m.m(i) = DEM::elevation(ll.at(i));
	DEM::unlock();

	return m;
}

void RasterTile::render()
{
	QImage img(_rect.width() * _ratio, _rect.height() * _ratio,
	  QImage::Format_ARGB32_Premultiplied);
	QList<MapData::Path> paths;
	QList<MapData::Point> points;

	fetchData(paths, points);

	QList<TextItem*> textItems;
	QVector<PainterPath> renderPaths(paths.size());

	img.setDevicePixelRatio(_ratio);
	img.fill(Qt::transparent);

	QPainter painter(&img);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.translate(-_rect.x(), -_rect.y());

	drawPaths(&painter, paths, points, renderPaths);

	processPointLabels(points, textItems);
	processAreaLabels(renderPaths, textItems);
	processLineLabels(renderPaths, textItems);
	drawTextItems(&painter, textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(_rect);

	qDeleteAll(textItems);

	_pixmap.convertFromImage(img);
}
