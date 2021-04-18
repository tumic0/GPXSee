#include <QPainter>
#include <QCache>
#include "common/programpaths.h"
#include "map/mapsforgemap.h"
#include "map/textpathitem.h"
#include "map/textpointitem.h"
#include "rastertile.h"

using namespace Mapsforge;

static const Style& style()
{
	static Style s(ProgramPaths::renderthemeFile());
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

static QString *pointLabel(const Style::TextRender *ri, MapData::Point &point)
{
	for (int i = 0; i < point.tags.size(); i++) {
		if (point.tags.at(i).key == ri->key()) {
			if (point.tags.at(i).value.isEmpty())
				return 0;
			else {
				point.label = point.tags.at(i).value;
				return &point.label;
			}
		}
	}

	return 0;
}

static QString *pathLabel(const Style::TextRender *ri, MapData::Path &path,
  bool *limit = 0)
{
	for (int i = 0; i < path.tags.size(); i++) {
		if (path.tags.at(i).key == ri->key()) {
			if (path.tags.at(i).value.isEmpty())
				return 0;
			else {
				path.label = path.tags.at(i).value;
				if (limit)
					*limit = (path.tags.at(i).key == "ref");
				return &path.label;
			}
		}
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
	const Style &s = style();
	QList<const Style::TextRender*> labels(s.pointLabels(_zoom));
	QList<const Style::Symbol*> symbols(s.pointSymbols(_zoom));

	for (int i = 0; i < _points.size(); i++) {
		MapData::Point &point = _points[i];
		QString *label = 0;
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(point.tags)) {
				if ((label = pointLabel(ri, point))) {
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

		TextPointItem *item = new TextPointItem(
		  ll2xy(point.coordinates).toPoint(), label, font, img, color,
		    hColor, 0, false);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processAreaLabels(QList<TextItem*> &textItems)
{
	const Style &s = style();
	QList<const Style::TextRender*> labels(s.areaLabels(_zoom));
	QList<const Style::Symbol*> symbols(s.areaSymbols(_zoom));

	for (int i = 0; i < _paths.size(); i++) {
		MapData::Path &path = _paths[i];
		QString *label = 0;
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;

		if (!path.closed || !path.path.elementCount())
			continue;

		for (int j = 0; j < labels.size(); j++) {
			const Style::TextRender *ri = labels.at(j);
			if (ri->rule().match(path.closed, path.tags)) {
				if ((label = pathLabel(ri, path))) {
					ti = ri;
					break;
				}
			}
		}

		for (int j = 0; j < symbols.size(); j++) {
			const Style::Symbol *ri = symbols.at(j);
			if (ri->rule().match(path.tags)) {
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
		QPointF pos = path.labelPos.isNull()
		  ? centroid(path.path) : ll2xy(path.labelPos);

		TextPointItem *item = new TextPointItem(pos.toPoint(), label, font, img,
		  color, hColor, 0, false);
		if (item->isValid() && _rect.contains(item->boundingRect().toRect())
		  && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;
	}
}

void RasterTile::processLineLabels(QList<TextItem*> &textItems)
{
	const Style &s = style();
	QList<const Style::TextRender*> instructions(s.pathLabels(_zoom));
	QSet<QString> set;

	for (int i = 0; i < instructions.size(); i++) {
		const Style::TextRender *ri = instructions.at(i);

		for (int j = 0; j < _paths.size(); j++) {
			MapData::Path &path = _paths[j];
			QString *label = 0;
			bool limit = false;

			if (!path.path.elementCount())
				continue;
			if (!ri->rule().match(path.closed, path.tags))
				continue;
			if (!(label = pathLabel(ri, path, &limit)))
				continue;
			if (limit && set.contains(path.label))
				continue;

			TextPathItem *item = new TextPathItem(path.path, label, _rect,
			  &ri->font(), &ri->fillColor(), haloColor(ri));
			if (item->isValid() && !item->collides(textItems)) {
				textItems.append(item);
				if (limit)
					set.insert(path.label);
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

QVector<RasterTile::PathInstruction> RasterTile::pathInstructions()
{
	QCache<Key, QVector<const Style::PathRender *> > cache(1024);
	QVector<PathInstruction> instructions;
	const Style &s = style();
	QVector<const Style::PathRender*> *ri;

	for (int i = 0 ; i < _paths.size(); i++) {
		MapData::Path &path = _paths[i];

		Key key(_zoom, path.closed, path.tags);
		QVector<const Style::PathRender*> *cached = cache.object(key);
		if (!cached) {
			ri = new QVector<const Style::PathRender*>(s.paths(_zoom,
			  path.closed, path.tags));
			for (int j = 0; j < ri->size(); j++)
				instructions.append(PathInstruction(ri->at(j), &path));
			cache.insert(key, ri);
		} else {
			for (int j = 0; j < cached->size(); j++)
				instructions.append(PathInstruction(cached->at(j), &path));
		}
	}

	std::sort(instructions.begin(), instructions.end());

	return instructions;
}

void RasterTile::drawPaths(QPainter *painter)
{
	QVector<PathInstruction> instructions(pathInstructions());
	const Style::PathRender *lri = 0;

	QPixmap layer(_pixmap.size());
	layer.setDevicePixelRatio(_ratio);
	layer.fill(Qt::transparent);

	QPainter lp(&layer);
	lp.setRenderHint(QPainter::Antialiasing);
	lp.translate(-_rect.x(), -_rect.y());
	lp.setCompositionMode(QPainter::CompositionMode_Source);

	for (int i = 0; i < instructions.size(); i++) {
		PathInstruction &is = instructions[i];
		const Style::PathRender *ri = is.render();

		if (lri && lri != ri) {
			painter->drawPixmap(_rect.topLeft(), layer);
			lp.fillRect(QRect(_rect.topLeft(), _pixmap.size()), Qt::transparent);
		}

		if (!is.path()->path.elementCount())
			is.path()->path = painterPath(is.path()->poly);

		if (ri->area()) {
			lp.setPen(ri->pen(_zoom));
			lp.setBrush(ri->brush());
			lp.drawPath(is.path()->path);
			lri = ri;
		} else {
			painter->setPen(ri->pen(_zoom));
			painter->setBrush(ri->brush());
			painter->drawPath(is.path()->path);
			lri = 0;
		}
	}

	if (lri)
		painter->drawPixmap(_rect.topLeft(), layer);
}

void RasterTile::render()
{
	std::sort(_points.begin(), _points.end());

	QList<TextItem*> textItems;

	_pixmap.fill(Qt::transparent);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.scale(_ratio, _ratio);
	painter.translate(-_rect.x(), -_rect.y());

	drawPaths(&painter);

	processPointLabels(textItems);
	processAreaLabels(textItems);
	processLineLabels(textItems);
	drawTextItems(&painter, textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(QRect(_rect.topLeft(), _pixmap.size()));

	qDeleteAll(textItems);
}
