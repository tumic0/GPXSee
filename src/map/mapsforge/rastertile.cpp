#include <QPainter>
#include <QCache>
#include "common/programpaths.h"
#include "map/mapsforgemap.h"
#include "map/textpathitem.h"
#include "rastertile.h"

using namespace Mapsforge;

static const Style& style(qreal ratio)
{
	static Style s(ProgramPaths::renderthemeFile(), ratio);
	return s;
}

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

static const QByteArray *label(const QByteArray &key,
  const QVector<MapData::Tag> &tags)
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
	const Style &s = style(_ratio);
	QList<const Style::TextRender*> labels(s.pointLabels(_zoom));
	QList<const Style::Symbol*> symbols(s.pointSymbols(_zoom));

	for (int i = 0; i < _points.size(); i++) {
		MapData::Point &point = _points[i];
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

		if (!ti && !si)
			continue;

		const QImage *img = si ? &si->img() : 0;
		const QFont *font = ti ? &ti->font() : 0;
		const QColor *color = ti ? &ti->fillColor() : 0;
		const QColor *hColor = ti ? haloColor(ti) : 0;

		PointItem *item = new PointItem(ll2xy(point.coordinates).toPoint(),
		  lbl ? new QString(*lbl) : 0, font, img, color, hColor);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processAreaLabels(QList<TextItem*> &textItems,
  QVector<RenderPath> &renderPaths)
{
	const Style &s = style(_ratio);
	QList<const Style::TextRender*> labels(s.areaLabels(_zoom));
	QList<const Style::Symbol*> symbols(s.areaSymbols(_zoom));

	for (int i = 0; i < renderPaths.size(); i++) {
		RenderPath &path = renderPaths[i];
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;

		if (!path.path->closed)
			continue;

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(path.path->closed, path.path->tags)) {
				const QByteArray *lbl;
				if ((lbl = label(ri->key(), path.path->tags))) {
					path.label = *lbl;
					ti = ri;
				}
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

		TextPointItem *item = new TextPointItem(pos.toPoint(), &path.label,
		  font, img, color, hColor, 0);
		if (item->isValid() && _rect.contains(item->boundingRect().toRect())
		  && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processLineLabels(QList<TextItem*> &textItems,
  QVector<RenderPath> &renderPaths)
{
	const Style &s = style(_ratio);
	QList<const Style::TextRender*> instructions(s.pathLabels(_zoom));
	QSet<QByteArray> set;

	for (int i = 0; i < instructions.size(); i++) {
		const Style::TextRender *ri = instructions.at(i);

		for (int i = 0; i < renderPaths.size(); i++) {
			RenderPath &path = renderPaths[i];
			const QByteArray *lbl = label(ri->key(), path.path->tags);

			if (!lbl)
				continue;
			if (!ri->rule().match(path.path->closed, path.path->tags))
				continue;
			bool limit = (ri->key() == "ref" || ri->key() == "ele"
			  || ri->key() == "ref_hike" || ri->key() == "ref_cycle"
			  || ri->key() == "ref_mtb");
			if (limit && set.contains(*lbl))
				continue;

			path.label = *lbl;

			TextPathItem *item = new TextPathItem(path.pp, &path.label, _rect,
			  &ri->font(), &ri->fillColor(), haloColor(ri));
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

QVector<RasterTile::PathInstruction> RasterTile::pathInstructions(
  QVector<RenderPath> &renderPaths)
{
	QCache<Key, QList<const Style::PathRender *> > cache(8192);
	QVector<PathInstruction> instructions;
	const Style &s = style(_ratio);
	QList<const Style::PathRender*> *ri;
	int i = 0;

	for (QSet<MapData::Path>::const_iterator it = _paths.cbegin();
	  it != _paths.cend(); ++it) {
		const MapData::Path &path = *it;
		RenderPath &rp = renderPaths[i];
		Key key(_zoom, path.closed, path.tags);

		rp.path = &path;

		if (!(ri = cache.object(key))) {
			ri = new QList<const Style::PathRender*>(s.paths(_zoom,
			  path.closed, path.tags));
			for (int j = 0; j < ri->size(); j++)
				instructions.append(PathInstruction(ri->at(j), &rp));
			cache.insert(key, ri);
		} else {
			for (int j = 0; j < ri->size(); j++)
				instructions.append(PathInstruction(ri->at(j), &rp));
		}

		i++;
	}

	std::sort(instructions.begin(), instructions.end());

	return instructions;
}

void RasterTile::drawPaths(QPainter *painter, QVector<RenderPath> &renderPaths)
{
	QVector<PathInstruction> instructions(pathInstructions(renderPaths));

	for (int i = 0; i < instructions.size(); i++) {
		const PathInstruction &is = instructions.at(i);
		const Style::PathRender *ri = is.render();
		RenderPath *path = is.path();

		if (!path->pp.elementCount())
			path->pp = painterPath(path->path->poly, ri->curve());

		painter->setPen(ri->pen(_zoom));
		painter->setBrush(ri->brush());
		painter->drawPath(path->pp);
	}
}

void RasterTile::render()
{
	std::sort(_points.begin(), _points.end());

	QList<TextItem*> textItems;
	QVector<RenderPath> renderPaths(_paths.size());

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
