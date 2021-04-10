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

void RasterTile::processPoints(QList<TextItem*> &textItems)
{
	const Style &s = style();

	for (int i = 0; i < _points.size(); i++) {
		const MapData::Point &point = _points.at(i);
		const QString *label = point.label.isEmpty() ? 0 : &(point.label);
		const Style::TextRender *ti = 0;
		const Style::Symbol *si = 0;

		if (label) {
			for (int i = 0; i < s.pointLabels().size(); i++) {
				const Style::TextRender &ri = s.pointLabels().at(i);
				if (ri.rule().match(_zoom, point.tags)) {
					ti = &ri;
					break;
				}
			}
		}

		for (int i = 0; i < s.symbols().size(); i++) {
			const Style::Symbol &ri = s.symbols().at(i);
			if (ri.rule().match(_zoom, point.tags)) {
				si = &ri;
				break;
			}
		}

		if (!ti && !si)
			continue;

		const QImage *img = si ? &si->img() : 0;
		const QFont *font = ti ? &ti->font() : 0;
		const QColor *color = ti ? &ti->fillColor() : 0;

		TextPointItem *item = new TextPointItem(
		  ll2xy(point.coordinates).toPoint(), label, font, img, color, 0, false);
		if (item->isValid() && !item->collides(textItems))
			textItems.append(item);
		else
			delete item;

	}
}

void RasterTile::processStreetNames(const QRect &tileRect,
  QList<TextItem*> &textItems)
{
	const Style &s = style();

	for (int i = 0; i < s.pathLabels().size(); i++) {
		const Style::TextRender &ri = s.pathLabels().at(i);

		for (int j = 0; j < _paths.size(); j++) {
			MapData::Path &path = _paths[j];

			if (path.label.isEmpty())
				continue;
			if (!path.path.elementCount())
				continue;
			if (!ri.rule().match(_zoom, path.closed, path.tags))
				continue;

			TextPathItem *item = new TextPathItem(path.path,
			  &path.label, tileRect, &ri.font(), &ri.fillColor(),
			  &ri.strokeColor());
			if (item->isValid() && !item->collides(textItems))
				textItems.append(item);
			else
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

	for (int i = 0 ; i < _paths.size(); i++) {
		MapData::Path &path = _paths[i];

		Key key(_zoom, path.closed, path.tags);
		QVector<const Style::PathRender*> *cached = cache.object(key);
		if (!cached) {
			QVector<const Style::PathRender*> *ri
			  = new QVector<const Style::PathRender*>();
			s.match(_zoom, path.closed, path.tags, ri);
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

	for (int i = 0; i < instructions.size(); i++) {
		PathInstruction &is = instructions[i];
		const Style::PathRender *ri = is.render();

		painter->setPen(ri->pen(_zoom));
		painter->setBrush(ri->brush());

		if (!is.path()->path.elementCount())
			is.path()->path = painterPath(is.path()->poly);

		painter->drawPath(is.path()->path);
	}
}

void RasterTile::render()
{
	std::sort(_points.begin(), _points.end());

	QList<TextItem*> textItems;
	QRect tileRect(_xy, _img.size());

	_img.fill(Qt::transparent);

	QPainter painter(&_img);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.translate(-_xy.x(), -_xy.y());

	drawPaths(&painter);

	processPoints(textItems);
	processStreetNames(tileRect, textItems);
	drawTextItems(&painter, textItems);

	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRect(QRect(_xy, _img.size()));

	qDeleteAll(textItems);
}
