#ifndef TEXT_H
#define TEXT_H

#include <QList>
#include <QRectF>
#include <QFont>
#include <QPen>
#include "map/textpointitem.h"
#include "map/textpathitem.h"
#include "vectortile.h"
#include "style.h"

class QImage;
class QPainterPath;
class QPainter;

namespace MVT {

class Text
{
public:
	Text(int zoom, int size, qreal ratio, const Style *style)
	  : _zoom(zoom), _ratio(ratio), _style(style),
	  _sceneRect(QRect(QPoint(0, 0), QSize(size, size))) {}

	void addLayer(const Style::Layer &style, VectorTile::Layer *data);
	void render(QPainter *painter) const;

private:
	struct Layer {
		Layer(const Style::Layer *style, VectorTile::Layer *data)
		  : style(style), data(data) {}

		const Style::Layer *style;
		VectorTile::Layer *data;
	};

	struct Properties {
		qreal maxWidth;
		qreal maxAngle;
		TextPointItem::Anchor anchor;
		Style::SymbolPlacement placement;
		Style::RotationAlignment alignment;
		QFont font;
		QColor color, haloColor;
	};

	struct CTX {
		CTX() : clip(false) {}
		~CTX() {qDeleteAll(items);}

		QList<TextItem*> items;
		bool clip;
	};

	class PointItem : public TextPointItem
	{
	public:
		PointItem(const QPointF &point, const QString &text, const QFont &font,
		  const QImage &img, const QColor &color, const QColor &haloColor,
		  TextPointItem::Anchor textAnchor, int maxWidth)
		  : TextPointItem(point.toPoint(), text.isEmpty() ? 0 : new QString(text),
		  text.isEmpty() ? 0 : new QFont(font), img.isNull() ? 0 : new QImage(img),
		  (color.isValid() && !text.isEmpty()) ? new QColor(color) : 0,
		  (haloColor.isValid() && !text.isEmpty()) ? new QColor(haloColor) : 0,
		  0, 0, NAN, textAnchor, maxWidth) {}
		~PointItem()
		{
			delete _text; delete _font; delete _img; delete _color;
			delete _haloColor;
		}
	};

	class PathItem : public TextPathItem
	{
	public:
		PathItem(const QPainterPath &line, const QString &text,
		  const QRect &tileRect, const QFont &font, const QColor &color,
		  const QColor &haloColor, qreal maxAngle)
		  : TextPathItem(line, text.isEmpty() ? 0 : new QString(text), tileRect,
		  text.isEmpty() ? 0 : new QFont(font), (color.isValid() && !text.isEmpty())
		  ? new QColor(color) : 0, (haloColor.isValid() && !text.isEmpty())
		  ? new QColor(haloColor) : 0, 0, true, maxAngle) {}
		~PathItem()
		  {delete _text; delete _font; delete _color; delete _haloColor;}
	};

	void addSymbols(CTX &ctx, const Layer &layer) const;
	void addSymbol(CTX &ctx, const Properties &prop, const Style::Layer &layer,
	  VectorTile::Feature &feature) const;
	TextItem *symbol(const Properties &prop, const QString &label,
	  const QImage &icon, const QPainterPath &path) const;

	int _zoom;
	qreal _ratio;
	const Style *_style;
	QRect _sceneRect;
	QList<Layer> _layers;
};

}

#endif // TEXT_H
