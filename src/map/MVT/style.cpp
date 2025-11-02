#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>
#include "common/programpaths.h"
#include "text.h"
#include "font.h"
#include "vectortile.h"
#include "style.h"

using namespace MVT;

static PBF::GeomType geometryType(const QString &str)
{
	if (str == "Point")
		return PBF::GeomType::POINT;
	else if (str == "LineString")
		return PBF::GeomType::LINESTRING;
	else if (str == "Polygon")
		return PBF::GeomType::POLYGON;
	else
		return PBF::GeomType::UNKNOWN;
}

static QVariant variant(const QJsonValue &val)
{
	switch (val.type()) {
		case QJsonValue::String:
			return QVariant(val.toString().toUtf8());
		case QJsonValue::Double:
		case QJsonValue::Bool:
			return val.toVariant();
		default:
			qWarning() << val << ": invalid filter value";
			return QVariant();
	}
}

Style::Layer::Filter::Filter(const QJsonArray &json)
  : _type(Unknown), _not(false)
{
#define INVALID_FILTER(json) \
	{qWarning() << json << ": invalid filter"; return;}

	if (json.isEmpty())
		INVALID_FILTER(json);

	QString type(json.at(0).toString());

	if (type == "==") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		if (json.at(1).toString() == "$type") {
			_type = GeometryType;
			_kv = QPair<QByteArray, QVariant>(QByteArray(),
			  QVariant(geometryType(json.at(2).toString())));
		} else {
			_type = EQ;
			_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
			  variant(json.at(2)));
		}
	} else if (type == "!=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = NE;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == "<") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = LT;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == "<=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = LE;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == ">") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = GT;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == ">=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = GE;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == "all") {
		_type = All;
		for (int i = 1; i < json.size(); i++)
			_filters.append(Filter(json.at(i).toArray()));
	} else if (type == "any") {
		_type = Any;
		for (int i = 1; i < json.size(); i++)
			_filters.append(Filter(json.at(i).toArray()));
	} else if (type == "in") {
		if (json.size() < 3)
			INVALID_FILTER(json);
		_type = In;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
		for (int i = 2; i < json.size(); i++)
			_set.append(variant(json.at(i)));
	}  else if (type == "!in") {
		if (json.size() < 3)
			INVALID_FILTER(json);
		_type = In;
		_not = true;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
		for (int i = 2; i < json.size(); i++)
			_set.append(variant(json.at(i)));
	} else if (type == "has") {
		if (json.size() < 2)
			INVALID_FILTER(json);
		_type = Has;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
	} else if (type == "!has") {
		if (json.size() < 2)
			INVALID_FILTER(json);
		_type = Has;
		_not = true;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
	} else
		INVALID_FILTER(json);
}

bool Style::Layer::Filter::match(const VectorTile::Feature &feature) const
{
	const QVariant *v;

	switch (_type) {
		case None:
			return true;
		case EQ:
			if (!(v = feature.value(_kv.first)))
				return false;
			else
				return *v == _kv.second;
		case NE:
			if (!(v = feature.value(_kv.first)))
				return true;
			else
				return *v != _kv.second;
		case GT:
			if (!(v = feature.value(_kv.first)))
				return false;
			else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v > _kv.second;
#else // QT6
				return (QVariant::compare(*v, _kv.second)
				  == QPartialOrdering::Greater);
#endif // QT6
		case GE:
			{if (!(v = feature.value(_kv.first)))
				return false;
			else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v >= _kv.second;
#else // QT6
				QPartialOrdering res = QVariant::compare(*v, _kv.second);
				return (res == QPartialOrdering::Greater
				  || res == QPartialOrdering::Equivalent);
#endif // QT6
			}}
		case LT:
			if (!(v = feature.value(_kv.first)))
				return false;
			else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v < _kv.second;
#else // QT6
				return (QVariant::compare(*v, _kv.second)
				  == QPartialOrdering::Less);
#endif // QT6
		case LE:
			{if (!(v = feature.value(_kv.first)))
				return false;
			else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v <= _kv.second;
#else // QT6
				QPartialOrdering res = QVariant::compare(*v, _kv.second);
				return (res == QPartialOrdering::Less
				  || res == QPartialOrdering::Equivalent);
#endif // QT6
			}}
		case In:
			if (!(v = feature.value(_kv.first)))
				return _not;
			else
				return _set.contains(*v) ^ _not;
		case Has:
			return (feature.value(_kv.first) ? true : false) ^ _not;
		case All:
			for (int i = 0; i < _filters.size(); i++)
				if (!_filters.at(i).match(feature))
					return false;
			return true;
		case Any:
			for (int i = 0; i < _filters.size(); i++)
				if (_filters.at(i).match(feature))
					return true;
			return false;
		case GeometryType:
			return feature.type() == _kv.second.toInt();
		default:
			return false;
	}
}

QString Style::Layer::Template::value(int zoom, const VectorTile::Feature &feature) const
{
	static QRegularExpression rx("\\{[^\\}]*\\}");
	QString text(_field.value(zoom));
	QRegularExpressionMatchIterator it = rx.globalMatch(text);
	QStringList keys;

	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QString val = match.captured(0);
		keys.append(val.mid(1, val.size() - 2));
	}
	for (int i = 0; i < keys.size(); i++) {
		const QString &key = keys.at(i);
		const QVariant *val = feature.value(key.toUtf8());
		text.replace(QString("{%1}").arg(key), val ? val->toString() : "");
	}

	return text;
}

Style::Layer::Paint::Paint(const QJsonObject &json)
{
	// fill
	_fillOpacity = FunctionF(json["fill-opacity"], 1.0);
	_fillColor = FunctionC(json["fill-color"]);
	if (json.contains("fill-outline-color"))
		_fillOutlineColor = FunctionC(json["fill-outline-color"]);
	else
		_fillOutlineColor = _fillColor;
	_fillAntialias = FunctionB(json["fill-antialias"]);
	if (json.contains("fill-pattern")) {
		_fillPattern = FunctionS(json["fill-pattern"]);
		_fillColor = FunctionC(QColor());
		_fillOutlineColor = FunctionC(QColor());
	}

	// line
	_lineColor = FunctionC(json["line-color"]);
	_lineWidth = FunctionF(json["line-width"], 1.0);
	_lineOpacity = FunctionF(json["line-opacity"], 1.0);
	if (json.contains("line-dasharray") && json["line-dasharray"].isArray()) {
		QJsonArray array = json["line-dasharray"].toArray();
		for (int i = 0; i < array.size(); i++)
			_lineDasharray.append(array.at(i).toDouble());
	}

	// background
	_backgroundColor = FunctionC(json["background-color"]);

	// text
	_textColor = FunctionC(json["text-color"]);
	_textHaloColor = FunctionC(json["text-halo-color"], QColor());

	// icon
	_iconColor = FunctionC(json["icon-color"]);
}

QPen Style::Layer::Paint::pen(Type type, int zoom) const
{
	QPen pen(Qt::NoPen);
	qreal width;
	QColor color;

	switch (type) {
		case Line:
			width = _lineWidth.value(zoom);
			color = _lineColor.value(zoom);
			if (color.isValid() && width > 0) {
				pen = QPen(color, width);
				if (!_lineDasharray.isEmpty())
					pen.setDashPattern(_lineDasharray);
			}
			break;
		case Fill:
			color = _fillOutlineColor.value(zoom);
			if (color.isValid())
				pen = QPen(color);
			break;
		case Symbol:
			color = _textColor.value(zoom);
			if (color.isValid())
				pen = QPen(color);
			break;
		default:
			break;
	}

	return pen;
}

QBrush Style::Layer::Paint::brush(Type type, int zoom, const Sprites &sprites)
  const
{
	QColor color;
	QBrush brush(Qt::NoBrush);
	QString pattern;

	switch (type) {
		case Fill:
			color = _fillColor.value(zoom);
			if (color.isValid())
				brush = QBrush(color);
			pattern = _fillPattern.value(zoom);
			if (!pattern.isNull())
				brush.setTextureImage(sprites.icon(pattern));
			break;
		case Background:
			color = _backgroundColor.value(zoom);
			if (color.isValid())
				brush = QBrush(color);
			pattern = _fillPattern.value(zoom);
			if (!pattern.isNull())
				brush.setTextureImage(sprites.icon(pattern));
			break;
		default:
			break;
	}

	return brush;
}

qreal Style::Layer::Paint::opacity(Type type, int zoom) const
{
	switch (type) {
		case Fill:
			return _fillOpacity.value(zoom);
		case Line:
			return _lineOpacity.value(zoom);
		default:
			return 1.0;
	}
}

bool Style::Layer::Paint::antialias(Layer::Type type, int zoom) const
{
	switch (type) {
		case Fill:
			return _fillAntialias.value(zoom);
		case Line:
			return true;
		default:
			return false;
	}
}

Style::Layer::Layout::Layout(const QJsonObject &json)
  : _lineCap(Qt::FlatCap), _lineJoin(Qt::MiterJoin), _font("Open Sans")
{
	// line
	_lineCap = FunctionS(json["line-cap"], "butt");
	_lineJoin = FunctionS(json["line-join"], "miter");

	// text
	_text = Template(FunctionS(json["text-field"]));

	_textSize = FunctionF(json["text-size"], 16);
	_textMaxWidth = FunctionF(json["text-max-width"], 10);
	_textMaxAngle = FunctionF(json["text-max-angle"], 45);
	_textTransform = FunctionS(json["text-transform"], "none");
	_textRotationAlignment = FunctionS(json["text-rotation-alignment"]);
	_textAnchor = FunctionS(json["text-anchor"]);

	if (json.contains("text-font") && json["text-font"].isArray())
		_font = Font::fromJsonArray(json["text-font"].toArray());

	// icon
	_icon = Template(FunctionS(json["icon-image"]));
	_iconSize = FunctionF(json["icon-size"], 1.0);

	// symbol
	_symbolPlacement = FunctionS(json["symbol-placement"]);

	// visibility
	if (json.contains("visibility") && json["visibility"].isString())
		_visible = !(json["visibility"].toString() == "none");
	else
		_visible = true;
}

QFont Style::Layer::Layout::font(int zoom) const
{
	QFont font(_font);
	font.setPixelSize(_textSize.value(zoom));
	font.setCapitalization(textTransform(zoom));

	return font;
}

TextPointItem::Anchor Style::Layer::Layout::textAnchor(int zoom) const
{
	QString anchor(_textAnchor.value(zoom));

	if (anchor == "left" || anchor == "top-left" || anchor == "bottom-left")
		return TextPointItem::Left;
	else if (anchor == "right" || anchor == "top-right"
	  || anchor == "bottom-right")
		return TextPointItem::Right;
	else if (anchor == "top")
		return TextPointItem::Top;
	else if (anchor == "bottom")
		return TextPointItem::Bottom;
	else
		return TextPointItem::Center;
}

QFont::Capitalization Style::Layer::Layout::textTransform(int zoom) const
{
	QString transform(_textTransform.value(zoom));

	if (transform == "uppercase")
		return QFont::AllUppercase;
	else if (transform == "lowercase")
		return QFont::AllLowercase;
	else
		return QFont::MixedCase;
}

Qt::PenCapStyle Style::Layer::Layout::lineCap(int zoom) const
{
	QString cap(_lineCap.value(zoom));

	if (cap == "round")
		return Qt::RoundCap;
	else if (cap == "square")
		return Qt::SquareCap;
	else
		return Qt::FlatCap;
}

Qt::PenJoinStyle Style::Layer::Layout::lineJoin(int zoom) const
{
	QString join(_lineJoin.value(zoom));

	if (join == "bevel")
		return Qt::BevelJoin;
	else if (join == "round")
		return Qt::RoundJoin;
	else
		return Qt::MiterJoin;
}

Style::SymbolPlacement Style::Layer::Layout::symbolPlacement(int zoom) const
{
	QString placement(_symbolPlacement.value(zoom));

	if (placement == "line")
		return Style::Line;
	else if (placement == "line-center")
		return Style::LineCenter;
	else
		return Style::Point;
}

Style::RotationAlignment Style::Layer::Layout::textRotationAlignment(int zoom)
  const
{
	QString alignment(_textRotationAlignment.value(zoom));

	if (alignment == "map")
		return Map;
	else if (alignment == "viewport")
		return Viewport;
	else
		return Auto;
}

Style::Layer::Layer(const QJsonObject &json)
  : _type(Unknown), _minZoom(0), _maxZoom(24)
{
	// type
	QString type(json["type"].toString());

	if (type == "fill")
		_type = Fill;
	else if (type == "line")
		_type = Line;
	else if (type == "background")
		_type = Background;
	else if (type == "symbol")
		_type = Symbol;
	else if (type == "hillshade")
		_type = Hillshade;

	// source-layer
	_sourceLayer = json["source-layer"].toString().toUtf8();

	// zooms
	if (json.contains("minzoom") && json["minzoom"].isDouble())
		_minZoom = json["minzoom"].toInt();
	if (json.contains("maxzoom") && json["maxzoom"].isDouble())
		_maxZoom = json["maxzoom"].toInt();

	// filter
	if (json.contains("filter") && json["filter"].isArray())
		_filter = Filter(json["filter"].toArray());

	// layout
	if (json.contains("layout") && json["layout"].isObject())
		_layout = Layout(json["layout"].toObject());

	// paint
	if (json.contains("paint") && json["paint"].isObject())
		_paint = Paint(json["paint"].toObject());
}

bool Style::Layer::match(int zoom) const
{
	return (zoom >= 0 && (zoom < _minZoom || zoom >= _maxZoom)) ? false : true;
}

bool Style::Layer::match(int zoom, const VectorTile::Feature &feature) const
{
	if (zoom >= 0 && (zoom < _minZoom || zoom >= _maxZoom))
		return false;

	return _filter.match(feature);
}

void Style::Layer::setPathPainter(int zoom, const Sprites &sprites,
  QPainter &painter) const
{
	QPen pen(_paint.pen(_type, zoom));
	pen.setJoinStyle(_layout.lineJoin(zoom));
	pen.setCapStyle(_layout.lineCap(zoom));

	QBrush brush(_paint.brush(_type, zoom, sprites));

	painter.setRenderHint(QPainter::Antialiasing, _paint.antialias(_type, zoom));
	painter.setPen(pen);
	painter.setBrush(brush);
	painter.setOpacity(_paint.opacity(_type, zoom));
}

void Style::Layer::setTextProperties(int zoom, qreal &maxWidth,
  qreal &maxAngle, TextPointItem::Anchor &anchor, QColor &color,
  QColor &haloColor, QFont &font, SymbolPlacement &symbolPlacement,
  RotationAlignment &rotationAlignment) const
{
	maxWidth = _layout.maxTextWidth(zoom);
	maxAngle = _layout.maxTextAngle(zoom);
	anchor = _layout.textAnchor(zoom);
	color = _paint.pen(_type, zoom).color();
	haloColor = _paint.haloColor(zoom);
	font = _layout.font(zoom);
	symbolPlacement = _layout.symbolPlacement(zoom);
	rotationAlignment = _layout.textRotationAlignment(zoom);
}

void Style::Layer::symbol(int zoom, const Sprites &sprites,
  VectorTile::Feature &feature, QString &label, QImage &img) const
{
	QString icon(_layout.icon(zoom, feature));
	QColor color(_paint.iconColor(zoom));
	qreal size(_layout.iconSize(zoom));

	label = _layout.text(zoom, feature);
	img = sprites.icon(icon, color, size);
}

static Sprites loadSprites(const QDir &styleDir, const QString &json,
  const QString &img)
{
	QString spritesJSON(styleDir.filePath(json));

	if (QFileInfo::exists(spritesJSON)) {
		QString spritesImg(styleDir.filePath(img));
		if (QFileInfo::exists(spritesImg))
			return Sprites(spritesJSON, spritesImg);
		else
			qWarning("%s: %s", qUtf8Printable(spritesImg), "no such file");
	}

	return Sprites();
}

Style::Style(const QString &fileName) : _valid(false)
{
	QFile file(fileName);
	if (!file.exists())
		return;
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning("%s: %s", qUtf8Printable(fileName),
		  qUtf8Printable(file.errorString()));
		return;
	}
	QByteArray ba(file.readAll());

	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(ba, &error));
	if (doc.isNull()) {
		qWarning("%s[%d]: %s", qUtf8Printable(fileName), error.offset,
		  qUtf8Printable(error.errorString()));
		return;
	}
	QJsonObject json(doc.object());

	_name = json["name"].toString();

	if (json.contains("layers") && json["layers"].isArray()) {
		QJsonArray layers = json["layers"].toArray();
		for (int i = 0; i < layers.size(); i++)
			if (layers[i].isObject())
				_layers.append(Layer(layers[i].toObject()));
	}

	QDir styleDir(QFileInfo(fileName).absoluteDir());
	_sprites = loadSprites(styleDir, "sprite.json", "sprite.png");
	_sprites2x = loadSprites(styleDir, "sprite@2x.json", "sprite@2x.png");

	_valid = true;
}

const Sprites &Style::sprites(qreal scale) const
{
	return (scale > 1.0 && !_sprites2x.isNull()) ? _sprites2x : _sprites;
}

QStringList Style::sourceLayers() const
{
	QSet<QString> set;

	for (int i = 0; i < _layers.size(); i++)
		if (!_layers.at(i).sourceLayer().isEmpty())
			set.insert(_layers.at(i).sourceLayer());

	return set.values();
}

bool Style::matches(const QStringList &layers) const
{
	QStringList sl(sourceLayers());

	for (int i = 0; i < sl.size(); i++)
		if (!layers.contains(sl.at(i)))
			return false;

	return true;
}

bool Style::hasHillShading() const
{
	for (int i = 0; i < _layers.size(); i++)
		if (_layers.at(i).type() == Layer::Hillshade)
			return true;

	return false;
}

QList<const Style*> Style::loadStyles(const QString &path)
{
	QDir dir(path);
	QFileInfoList styles(dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot));
	QList<const Style*> list;

	for (int i = 0; i < styles.size(); i++) {
		QDir d(styles.at(i).absoluteFilePath());
		Style *style = new Style(d.filePath("style.json"));
		if (style->isValid())
			list.append(style);
		else
			delete style;
	}

	return list;
}

QList<const Style*> Style::loadStyles()
{
	QList<const Style*> list;

	QString dir(ProgramPaths::styleDir());
	if (!dir.isEmpty())
		list = loadStyles(dir);

	return list;
}

QList<const Style*> &Style::styles()
{
	static QList<const Style*> list = loadStyles();
	return list;
}
