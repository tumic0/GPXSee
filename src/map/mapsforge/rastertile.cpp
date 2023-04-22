#include <QPainter>
#include <QCache>
#include "common/programpaths.h"
#include "rastertile.h"

using namespace Mapsforge;

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

void RasterTile::processPointLabels(QList<TextItem*> &textItems)
{
	QList<const Style::TextRender*> labels(_style->pointLabels(_zoom));
	QList<const Style::Symbol*> symbols(_style->pointSymbols(_zoom));
	QList<PainterPoint> points;

	for (int i = 0; i < _points.size(); i++) {
		const MapData::Point &point = _points.at(i);
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
			points.append(PainterPoint(&point, lbl, si, ti));
	}

	std::sort(points.begin(), points.end());

	for (int i = 0; i < points.size(); i++) {
		const PainterPoint &p = points.at(i);
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

	for (int i = 0; i < polygon.size(); i++) {
		const QVector<Coordinates> &subpath = polygon.at(i);

		if (curve) {
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
		} else {
			path.moveTo(ll2xy(subpath.first()));
			for (int j = 1; j < subpath.size(); j++)
				path.lineTo(ll2xy(subpath.at(j)));
		}
	}

	return path;
}

void RasterTile::pathInstructions(QVector<PainterPath> &paths,
  QVector<RasterTile::RenderInstruction> &instructions)
{
	QCache<PathKey, QList<const Style::PathRender *> > cache(8192);
	QList<const Style::PathRender*> *ri;

	for (int i = 0; i < _paths.size(); i++) {
		const MapData::Path &path = _paths.at(i);
		PainterPath &rp = paths[i];
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

void RasterTile::circleInstructions(
  QVector<RasterTile::RenderInstruction> &instructions)
{
	QCache<PointKey, QList<const Style::CircleRender *> > cache(8192);
	QList<const Style::CircleRender*> *ri;

	for (int i = 0; i < _points.size(); i++) {
		const MapData::Point &point = _points.at(i);
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

void RasterTile::drawPaths(QPainter *painter, QVector<PainterPath> &paths)
{
	QVector<RenderInstruction> instructions;
	pathInstructions(paths, instructions);
	circleInstructions(instructions);
	std::sort(instructions.begin(), instructions.end());

	for (int i = 0; i < instructions.size(); i++) {
		const RenderInstruction &is = instructions.at(i);
		PainterPath *path = is.path();

		if (path) {
			const Style::PathRender *ri = is.pathRender();

			if (!path->pp.elementCount())
				path->pp = painterPath(path->path->poly, ri->curve());

			painter->setPen(ri->pen(_zoom));
			painter->setBrush(ri->brush());
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

void RasterTile::render()
{
	QList<TextItem*> textItems;
	QVector<PainterPath> renderPaths(_paths.size());

	_pixmap.setDevicePixelRatio(_ratio);
	_pixmap.fill(Qt::transparent);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_rect.x(), -_rect.y());

	drawPaths(&painter, renderPaths);

	processPointLabels(textItems);
	processAreaLabels(textItems, renderPaths);
	processLineLabels(textItems, renderPaths);
	drawTextItems(&painter, textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(QRect(_rect.topLeft(), _pixmap.size()));

	qDeleteAll(textItems);

	_valid = true;
}
