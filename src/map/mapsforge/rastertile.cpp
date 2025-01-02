#include <cmath>
#include <QPainter>
#include <QCache>
#include "map/dem.h"
#include "map/rectd.h"
#include "map/hillshading.h"
#include "map/filter.h"
#include "map/bitmapline.h"
#include "rastertile.h"

using namespace Mapsforge;

#define TEXT_EXTENT 160
#define PATHS_EXTENT 20
#define SEARCH_EXTENT -0.5

static double LIMIT = cos(deg2rad(170));

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

void RasterTile::processLabels(const QList<MapData::Point> &points,
  QList<TextItem*> &textItems) const
{
	QList<Label> items;
	QList<const Style::TextRender*> labels(_style->labels(_zoom));
	QList<const Style::Symbol*> symbols(_style->symbols(_zoom));

	for (int i = 0; i < points.size(); i++) {
		const MapData::Point &point = points.at(i);
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;
		QList<const QByteArray *> ll;

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);
			if (ri->rule().match(point.center(), point.tags)) {
				si = ri;
				break;
			}
		}

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(point.center(), point.tags)) {
				const QByteArray *lbl = label(ri->key(), point.tags);
				if (lbl) {
					if (!si) {
						ti = ri;
						ll.append(lbl);
						break;
					} else if (si->id() == ri->symbolId()) {
						if (!ti)
							ti = ri;
						ll.append(lbl);
					}
				}
			}
		}

		if (ti || si)
			items.append(Label(&point, ll, si, ti));
	}

	std::sort(items.begin(), items.end());

	for (int i = 0; i < items.size(); i++) {
		const Label &l = items.at(i);
		const QImage *img = l.si ? &l.si->img() : 0;
		const QFont *font = l.ti ? &l.ti->font() : 0;
		const QColor *color = l.ti ? &l.ti->fillColor() : 0;
		const QColor *hColor = l.ti ? haloColor(l.ti) : 0;

		PointItem *item = new PointItem(ll2xy(l.point->coordinates).toPoint(),
		  l.lbl, font, img, color, hColor);
		if (item->isValid() && !item->collides(textItems))
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
	QList<LineLabel> items;
	QSet<QByteArray> set;

	for (int i = 0; i < paths.size(); i++) {
		const PainterPath &path = paths.at(i);
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;
		const QByteArray *lbl = 0;

		if (path.path->closed)
			continue;

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);
			if (ri->rule().matchPath(path.path->closed, path.path->point.tags)) {
				si = ri;
				break;
			}
		}

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().matchPath(path.path->closed, path.path->point.tags)) {
				if ((lbl = label(ri->key(), path.path->point.tags))) {
					if (!si || si->id() == ri->symbolId()) {
						ti = ri;
						break;
					}
				}
			}
		}

		if (ti || si)
			items.append(LineLabel(&path, lbl, si, ti));
	}

	std::sort(items.begin(), items.end());

	for (int i = 0; i < items.size(); i++) {
		const LineLabel &l = items.at(i);
		const QImage *img = l.si ? &l.si->img() : 0;
		const QFont *font = l.ti ? &l.ti->font() : 0;
		const QColor *color = l.ti ? &l.ti->fillColor() : 0;
		const QColor *hColor = l.ti ? haloColor(l.ti) : 0;
		bool rotate = l.si ? l.si->rotate() : false;
		bool limit = false;

		if (l.ti) {
			limit = (l.ti->key() == ID_ELE || l.ti->key() == ID_REF
			  || (!l.si && l.ti->shield()));
			if (limit && set.contains(*l.lbl))
				continue;
		}

		if (!l.si && l.ti && l.ti->shield()) {
			if (l.ti && l.lbl && set.contains(*l.lbl))
				continue;
			if (l.path->pp.length() < _rect.width() / 3.0)
				continue;

			QPointF pos = l.path->pp.pointAtPercent(0.5);

			PointItem *item = new PointItem(pos.toPoint(), l.lbl, font, color,
			  hColor);
			if (item->isValid() && _rect.contains(item->boundingRect().toRect())
			  && !item->collides(textItems)) {
				textItems.append(item);
				if (l.ti && l.lbl)
					set.insert(*l.lbl);
			} else
				delete item;
		} else {
			PathItem *item = new PathItem(l.path->pp, l.lbl, img, _rect, font,
			  color, hColor, rotate);
			if (item->isValid() && !item->collides(textItems)) {
				textItems.append(item);
				if (limit)
					set.insert(*l.lbl);
			} else {
				delete item;

				if (img && l.lbl) {
					PathItem *item = new PathItem(l.path->pp, 0, img, _rect, 0,
					  0, 0, rotate);
					if (item->isValid() && !item->collides(textItems))
						textItems.append(item);
					else
						delete item;
				}
			}
		}
	}
}

void RasterTile::drawTextItems(QPainter *painter,
  const QList<TextItem*> &textItems)
{
	QRectF rect(_rect);

	for (int i = 0; i < textItems.size(); i++) {
		const TextItem *ti = textItems.at(i);
		if (rect.intersects(ti->boundingRect()))
			ti->paint(painter);
	}
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
		PathKey key(_zoom, path.closed, path.point.tags);

		rp.path = &path;

		if (!(ri = cache.object(key))) {
			ri = new QList<const Style::PathRender*>(_style->paths(_zoom,
			  path.closed, path.point.tags));
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

			if (ri->bitmapLine()) {
				if (dy != 0)
					BitmapLine::draw(painter, parallelPath(path->pp, dy),
					  ri->img());
				else
					BitmapLine::draw(painter, path->pp, ri->img());
			} else {
				painter->setPen(ri->pen(_zoom));
				painter->setBrush(ri->brush());

				if (dy != 0)
					painter->drawPath(parallelPath(path->pp, dy));
				else
					painter->drawPath(path->pp);
			}
		} else if (point) {
			const Style::CircleRender *ri = is.circleRender();
			qreal radius = ri->radius(_zoom);

			painter->setPen(ri->pen());
			painter->setBrush(ri->brush());
			painter->drawEllipse(ll2xy(point->coordinates), radius, radius);
		} else {
			if (_hillShading) {
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
	}
}

void RasterTile::fetchData(QList<MapData::Path> &paths,
  QList<MapData::Point> &points) const
{
	QPoint ttl(_rect.topLeft());
	QFile file(_data->fileName());

	if (!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
		qWarning("%s: %s", qPrintable(file.fileName()),
		  qPrintable(file.errorString()));
		return;
	}

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
	_data->paths(file, searchRectD.toRectC(_proj, 20),
	  pathRectD.toRectC(_proj, 20), _zoom, &paths);

	QRectF pointRect(QPointF(ttl.x() - TEXT_EXTENT, ttl.y() - TEXT_EXTENT),
	  QPointF(ttl.x() + _rect.width() + TEXT_EXTENT, ttl.y() + _rect.height()
	  + TEXT_EXTENT));
	RectD pointRectD(_transform.img2proj(pointRect.topLeft()),
	  _transform.img2proj(pointRect.bottomRight()));
	_data->points(file, pointRectD.toRectC(_proj, 20), _zoom, &points);
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

	return DEM::elevation(ll);
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

	processLabels(points, textItems);
	processLineLabels(renderPaths, textItems);
	drawTextItems(&painter, textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(_rect);

	qDeleteAll(textItems);

	_pixmap.convertFromImage(img);
}
