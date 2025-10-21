#include <QFontMetrics>
#include <QPainter>
#include <QSet>
#include <QMap>
#include "text.h"

using namespace MVT;

void Text::addLayer(const Style::Layer *style, Tile::Layer *data)
{
	_layers.append(Layer(style, data));
}

void Text::render(QPainter *painter) const
{
	CTX ctx;
	QRectF rect(_sceneRect);

	for (int i = _layers.size() - 1; i >= 0; i--)
		addSymbols(ctx, _layers.at(i));

	for (int i = 0; i < ctx.items.size(); i++) {
		const TextItem *ti = ctx.items.at(i);
		if (rect.intersects(ti->boundingRect()))
			ti->paint(painter);
	}
}

void Text::addSymbols(CTX &ctx, const Layer &layer) const
{
	Properties prop;

	layer.style->setTextProperties(_zoom, prop.maxWidth, prop.maxAngle,
	  prop.anchor, prop.color, prop.haloColor, prop.font, prop.placement,
	  prop.alignment);

	if (prop.placement != Style::Point)
		ctx.clip = true;

	for (int i = 0; i < layer.data->features().size(); i++)
		addSymbol(ctx, prop, *layer.style, layer.data->features()[i]);
}

void Text::addSymbol(CTX &ctx, const Properties &prop, const Style::Layer &layer,
  Tile::Feature &feature) const
{
	if (!layer.match(_zoom, feature))
		return;

	QString label;
	QImage icon;
	const QPainterPath &path = feature.path(_sceneRect.width());
	layer.symbol(_zoom, _style->sprites(_ratio), feature, label, icon);

	TextItem *ti = symbol(prop, label, icon, path);
	if (!ti || !ti->isValid()) {
		delete ti;
		return;
	}
	// Note: empty path == point geometry (single move)
	if ((!path.isEmpty() || ctx.clip)
	  && !_sceneRect.contains(ti->boundingRect().toRect())) {
		delete ti;
		return;
	}
	if (ti->collides(ctx.items)) {
		delete ti;
		return;
	}

	ctx.items.append(ti);
}

TextItem *Text::symbol(const Properties &prop, const QString &label,
  const QImage &icon, const QPainterPath &path) const
{
	TextItem *ti;

	if (prop.alignment == Style::Viewport) {
		QMap<qreal, int> map;
		for (int j = 0; j < path.elementCount(); j++) {
			QLineF l(path.elementAt(j), _sceneRect.center());
			map.insert(l.length(), j);
		}
		QMap<qreal, int>::const_iterator jt = map.constBegin();
		ti = new PointItem(path.elementAt(jt.value()), label, prop.font,
		  icon, prop.color, prop.haloColor, prop.anchor, prop.maxWidth);
		while (true) {
			if (_sceneRect.contains(ti->boundingRect().toRect()))
				break;
			if (++jt == map.constEnd())
				break;
			static_cast<TextPointItem*>(ti)->setPos(QPointF(path.elementAt(
			  jt.value())).toPoint());
		}
	} else {
		switch (prop.placement) {
			case Style::Line:
				if (label.isEmpty())
					return 0;
				ti = new PathItem(path, label, _sceneRect, prop.font, prop.color,
				  prop.haloColor, prop.maxAngle);
				break;
			case Style::LineCenter:
				ti = new PointItem(path.pointAtPercent(0.5), label, prop.font,
				  icon, prop.color, prop.haloColor, prop.anchor, prop.maxWidth);
				break;
			default:
				ti = new PointItem(path.elementAt(0), label, prop.font,
				  icon, prop.color, prop.haloColor, prop.anchor, prop.maxWidth);
		}
	}

	return ti;
}
