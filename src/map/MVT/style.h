#ifndef MVT_STYLE_H
#define MVT_STYLE_H

#include <QVector>
#include <QPair>
#include <QStringList>
#include <QPen>
#include <QBrush>
#include <QFont>
#include "map/textpointitem.h"
#include "tile.h"
#include "function.h"
#include "sprites.h"

class QPainter;
class QPainterPath;
class QJsonArray;
class QJsonObject;

namespace MVT {

class Style
{
public:
	enum SymbolPlacement {
		Point,
		Line,
		LineCenter
	};

	enum RotationAlignment {
		Map,
		Viewport,
		Auto
	};

	class Layer {
	public:
		enum Type {
			Unknown,
			Fill,
			Line,
			Background,
			Symbol,
			Hillshade
		};

		Layer() : _type(Unknown), _minZoom(0), _maxZoom(24) {}
		Layer(const QJsonObject &json);

		const QByteArray &sourceLayer() const {return _sourceLayer;}
		Type type() const {return _type;}
		bool isVisible() const {return (_layout.visible());}

		bool match(int zoom) const;
		bool match(int zoom, const Tile::Feature &feature) const;
		void setPathPainter(int zoom, const Sprites &sprites,
		  QPainter &painter) const;
		void setTextProperties(int zoom, qreal &maxWidth, qreal &maxAngle,
		  TextPointItem::Anchor &anchor, QColor &color, QColor &haloColor,
		  QFont &font, SymbolPlacement &symbolPlacement,
		  RotationAlignment &rotationAlignment) const;
		void symbol(int zoom, const Sprites &sprites,
		  Tile::Feature &feature, QString &label, QImage &img) const;

	private:
		class Filter {
		public:
			Filter() : _type(None) {}
			Filter(const QJsonArray &json);

			bool match(const Tile::Feature &feature) const;
		private:
			enum Type {
				None, Unknown,
				EQ, NE, GE, GT, LE, LT,
				All, Any,
				In, Has, GeometryType
			};

			Type _type;
			bool _not;
			QVector<QVariant> _set;
			QPair<QByteArray, QVariant> _kv;
			QVector<Filter> _filters;
		};

		class Template {
		public:
			Template() {}
			Template(const FunctionS &str) : _field(str) {}
			QString value(int zoom, const Tile::Feature &feature) const;

		private:
			FunctionS _field;
		};

		class Layout {
		public:
			Layout() : _iconSize(1.0), _textSize(16), _textMaxWidth(10),
			  _textMaxAngle(45), _font("Open Sans"), _visible(true) {}
			Layout(const QJsonObject &json);

			qreal maxTextWidth(int zoom) const
			  {return _textMaxWidth.value(zoom);}
			qreal maxTextAngle(int zoom) const
			  {return _textMaxAngle.value(zoom);}
			QString text(int zoom, const Tile::Feature &feature) const
			  {return _text.value(zoom, feature).trimmed();}
			QString icon(int zoom, const Tile::Feature &feature) const
			  {return _icon.value(zoom, feature);}
			qreal iconSize(int zoom) const
			  {return _iconSize.value(zoom);}
			QFont font(int zoom) const;
			Qt::PenCapStyle lineCap(int zoom) const;
			Qt::PenJoinStyle lineJoin(int zoom) const;
			TextPointItem::Anchor textAnchor(int zoom) const;
			SymbolPlacement symbolPlacement(int zoom) const;
			RotationAlignment textRotationAlignment(int zoom) const;
			bool visible() const {return _visible;}

		private:
			QFont::Capitalization textTransform(int zoom) const;

			Template _text;
			Template _icon;
			FunctionF _iconSize;
			FunctionF _textSize;
			FunctionF _textMaxWidth;
			FunctionF _textMaxAngle;
			FunctionS _lineCap;
			FunctionS _lineJoin;
			FunctionS _textAnchor;
			FunctionS _textTransform;
			FunctionS _symbolPlacement;
			FunctionS _textRotationAlignment;
			QFont _font;
			bool _visible;
		};

		class Paint {
		public:
			Paint() : _fillOpacity(1.0), _lineOpacity(1.0), _lineWidth(1.0) {}
			Paint(const QJsonObject &json);

			QPen pen(Layer::Type type, int zoom) const;
			QBrush brush(Layer::Type type, int zoom, const Sprites &sprites)
			  const;
			qreal opacity(Layer::Type type, int zoom) const;
			bool antialias(Layer::Type type, int zoom) const;
			QColor haloColor(int zoom) const
			  {return _textHaloColor.value(zoom);}
			QColor iconColor(int zoom) const
			  {return _iconColor.value(zoom);}

		private:
			FunctionC _textColor;
			FunctionC _textHaloColor;
			FunctionC _lineColor;
			FunctionC _fillColor;
			FunctionC _fillOutlineColor;
			FunctionC _backgroundColor;
			FunctionC _iconColor;
			FunctionF _fillOpacity;
			FunctionF _lineOpacity;
			FunctionF _lineWidth;
			FunctionB _fillAntialias;
			QVector<qreal> _lineDasharray;
			FunctionS _fillPattern;
		};

		Type _type;
		QByteArray _sourceLayer;
		int _minZoom, _maxZoom;
		Filter _filter;
		Layout _layout;
		Paint _paint;
	};

	Style(const QString &fileName);

	bool isValid() const {return _valid;}
	const QString &name() const {return _name;}
	const QVector<Layer> &layers() const {return _layers;}
	const Sprites &sprites(qreal scale) const;

	bool matches(const QStringList &layers) const;
	bool hasHillShading() const;

	static QList<const Style*> &styles();

private:
	QStringList sourceLayers() const;

	static QList<const Style*> loadStyles(const QString &path);
	static QList<const Style*> loadStyles();

	QString _name;
	QVector<Layer> _layers;
	Sprites _sprites, _sprites2x;
	bool _valid;
};

}

#endif // MVT_STYLE_H
