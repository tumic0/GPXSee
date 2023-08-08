#include <QFile>
#include <QXmlStreamReader>
#include <QUrl>
#include <QFileInfo>
#include <QImageReader>
#include "common/programpaths.h"
#include "style.h"

using namespace Mapsforge;

static QString resourcePath(const QString &src, const QString &dir)
{
	QUrl url(src);
	if (url.scheme().isEmpty())
		return src;
	else
		return dir + "/" + url.toLocalFile();
}

static QImage image(const QString &path, int width, int height, int percent,
  qreal ratio)
{
	QImageReader ir(path, "svg");

	if (ir.canRead()) {
		QSize s(ir.size());

		if (!height && !width) {
			height = 20;
			width = 20;
		} else if (!width) {
			width = s.height() / (s.height() / (double)height);
		} else if (!height)
			height = s.width() / (s.width() / (double)width);

		if (percent != 100) {
			width *= percent / 100.0;
			height *= percent / 100.0;
		}

		ir.setScaledSize(QSize(width * ratio, height * ratio));
		QImage img(ir.read());
		img.setDevicePixelRatio(ratio);
		return img;
	} else
		return QImage(path);
}

static QList<unsigned> keyList(const MapData &data, const QList<QByteArray> &in)
{
	QList<unsigned> out;

	for (int i = 0; i < in.size(); i++) {
		if (in.at(i) == "*")
			out.append(0);
		else {
			unsigned key = data.tagId(in.at(i));
			if (key)
				out.append(key);
		}
	}

	return out;
}

static QList<QByteArray> valList(const QList<QByteArray> &in)
{
	QList<QByteArray> out;

	for (int i = 0; i < in.size(); i++) {
		if (in.at(i) == "*")
			out.append(QByteArray());
		else
			out.append(in.at(i));
	}

	return out;
}

const Style::Menu::Layer *Style::Menu::findLayer(const QString &id) const
{
	for (int i = 0; i < _layers.size(); i++)
		if (_layers.at(i).id() == id)
			return &_layers.at(i);

	qWarning("%s: layer not found", qPrintable(id));

	return 0;
}

void Style::Menu::addCats(const Layer *layer, QSet<QString> &cats) const
{
	if (!layer)
		return;

	if (!layer->parent().isNull())
		addCats(findLayer(layer->parent()), cats);

	for (int i = 0; i < layer->cats().size(); i++)
		cats.insert(layer->cats().at(i));

	for (int i = 0; i < layer->overlays().size(); i++) {
		const Layer *overlay = findLayer(layer->overlays().at(i));
		if (overlay && overlay->enabled())
			addCats(overlay, cats);
	}
}

QSet<QString> Style::Menu::cats() const
{
	QSet<QString> c;
	addCats(findLayer(_defaultvalue), c);
	return c;
}

Style::Rule::Filter::Filter(const MapData &data, const QList<QByteArray> &keys,
  const QList<QByteArray> &vals) : _neg(false)
{
	_keys = keyList(data, keys);

	QList<QByteArray> vc(vals);
	if (vc.removeAll("~"))
		_neg = true;
	_vals = valList(vc);
}

bool Style::Rule::match(const QVector<MapData::Tag> &tags) const
{
	for (int i = 0; i < _filters.size(); i++)
		if (!_filters.at(i).match(tags))
			return false;

	return true;
}

bool Style::Rule::match(bool closed, const QVector<MapData::Tag> &tags) const
{
	Closed cl = closed ? YesClosed : NoClosed;

	if (_closed && cl != _closed)
		return false;

	for (int i = 0; i < _filters.size(); i++)
		if (!_filters.at(i).match(tags))
			return false;

	return true;
}

bool Style::Rule::match(int zoom, bool closed,
  const QVector<MapData::Tag> &tags) const
{
	Closed cl = closed ? YesClosed : NoClosed;

	if (!_zooms.contains(zoom))
		return false;
	if (_closed && cl != _closed)
		return false;

	for (int i = 0; i < _filters.size(); i++)
		if (!_filters.at(i).match(tags))
			return false;

	return true;
}

bool Style::Rule::match(int zoom, const QVector<MapData::Tag> &tags) const
{
	if (!_zooms.contains(zoom))
		return false;

	for (int i = 0; i < _filters.size(); i++)
		if (!_filters.at(i).match(tags))
			return false;

	return true;
}

void Style::area(QXmlStreamReader &reader, const QString &dir, qreal ratio,
  qreal baseStrokeWidth, const Rule &rule)
{
	PathRender ri(rule, _paths.size() + _circles.size());
	const QXmlStreamAttributes &attr = reader.attributes();
	QString file;
	QColor fillColor;
	int height = 0, width = 0, percent = 100;
	bool ok;

	ri._area = true;
	if (attr.hasAttribute("fill"))
		fillColor = QColor(attr.value("fill").toString());
	if (attr.hasAttribute("stroke"))
		ri._strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("stroke-width")) {
		ri._strokeWidth = attr.value("stroke-width").toFloat(&ok)
		  * baseStrokeWidth;
		if (!ok || ri._strokeWidth < 0) {
			reader.raiseError("invalid stroke-width value");
			return;
		}
	}
	if (attr.hasAttribute("scale")) {
		QString scale(attr.value("scale").toString());
		if (scale == "all")
			ri._scale = PathRender::Scale::All;
		else if (scale == "none")
			ri._scale = PathRender::Scale::None;
	}
	if (attr.hasAttribute("src"))
		file = resourcePath(attr.value("src").toString(), dir);
	if (attr.hasAttribute("symbol-height")) {
		height = attr.value("symbol-height").toInt(&ok);
		if (!ok || height < 0) {
			reader.raiseError("invalid symbol-height value");
			return;
		}
	}
	if (attr.hasAttribute("symbol-width")) {
		width = attr.value("symbol-width").toInt(&ok);
		if (!ok || width < 0) {
			reader.raiseError("invalid symbol-width value");
			return;
		}
	}
	if (attr.hasAttribute("symbol-percent")) {
		percent = attr.value("symbol-percent").toInt(&ok);
		if (!ok || percent < 0) {
			reader.raiseError("invalid symbol-percent value");
			return;
		}
	}

	if (!file.isNull())
		ri._brush = QBrush(image(file, width, height, percent, ratio));
	else if (fillColor.isValid())
		ri._brush = QBrush(fillColor);

	if (ri.rule()._type == Rule::AnyType || ri.rule()._type == Rule::WayType)
		_paths.append(ri);

	reader.skipCurrentElement();
}

void Style::line(QXmlStreamReader &reader, qreal baseStrokeWidth,
  const Rule &rule)
{
	PathRender ri(rule, _paths.size() + _circles.size());
	const QXmlStreamAttributes &attr = reader.attributes();
	bool ok;

	ri._brush = Qt::NoBrush;

	if (attr.hasAttribute("stroke"))
		ri._strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("stroke-width")) {
		ri._strokeWidth = attr.value("stroke-width").toFloat(&ok)
		  * baseStrokeWidth;
		if (!ok || ri._strokeWidth < 0) {
			reader.raiseError("invalid stroke-width value");
			return;
		}
	}
	if (attr.hasAttribute("stroke-dasharray")) {
		QStringList l(attr.value("stroke-dasharray").toString().split(','));
		ri._strokeDasharray.resize(l.size());
		for (int i = 0; i < l.size(); i++) {
			ri._strokeDasharray[i] = l.at(i).toDouble(&ok);
			if (!ok || ri._strokeDasharray[i] < 0) {
				reader.raiseError("invalid stroke-dasharray value");
				return;
			}
		}
	}
	if (attr.hasAttribute("stroke-linecap")) {
		QString cap(attr.value("stroke-linecap").toString());
		if (cap == "butt")
			ri._strokeCap = Qt::FlatCap;
		else if (cap == "round")
			ri._strokeCap = Qt::RoundCap;
		else if (cap == "square")
			ri._strokeCap = Qt::SquareCap;
	}
	if (attr.hasAttribute("stroke-linejoin")) {
		QString join(attr.value("stroke-linejoin").toString());
		if (join == "miter")
			ri._strokeJoin = Qt::MiterJoin;
		else if (join == "round")
			ri._strokeJoin = Qt::RoundJoin;
		else if (join == "bevel")
			ri._strokeJoin = Qt::BevelJoin;
	}
	if (attr.hasAttribute("scale")) {
		QString scale(attr.value("scale").toString());
		if (scale == "all")
			ri._scale = PathRender::Scale::All;
		else if (scale == "none")
			ri._scale = PathRender::Scale::None;
	}
	if (attr.hasAttribute("curve")) {
		QString curve(attr.value("curve").toString());
		if (curve == "cubic")
			ri._curve = true;
	}
	if (attr.hasAttribute("dy")) {
		ri._dy = attr.value("dy").toDouble(&ok) * baseStrokeWidth;
		if (!ok) {
			reader.raiseError("invalid dy value");
			return;
		}
	}

	if (ri.rule()._type == Rule::AnyType || ri.rule()._type == Rule::WayType)
		_paths.append(ri);

	reader.skipCurrentElement();
}

void Style::circle(QXmlStreamReader &reader, qreal baseStrokeWidth,
  const Rule &rule)
{
	CircleRender ri(rule, _paths.size() + _circles.size());
	const QXmlStreamAttributes &attr = reader.attributes();
	bool ok;
	QColor fillColor, strokeColor;
	qreal strokeWidth = 0;

	if (attr.hasAttribute("fill"))
		fillColor = QColor(attr.value("fill").toString());
	if (attr.hasAttribute("stroke"))
		strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("stroke-width")) {
		strokeWidth = attr.value("stroke-width").toFloat(&ok) * baseStrokeWidth;
		if (!ok || strokeWidth < 0) {
			reader.raiseError("invalid stroke-width value");
			return;
		}
	}
	if (attr.hasAttribute("radius")) {
		ri._radius = attr.value("radius").toDouble(&ok) * baseStrokeWidth;
		if (!ok || ri._radius <= 0) {
			reader.raiseError("invalid radius value");
			return;
		}
	} else {
		reader.raiseError("missing radius");
		return;
	}
	if (attr.hasAttribute("scale-radius")) {
		if (attr.value("scale-radius").toString() == "true")
			ri._scale = true;
	}

	ri._pen = (strokeColor.isValid() && strokeWidth > 0)
	  ? QPen(QBrush(strokeColor), strokeWidth) : Qt::NoPen;
	ri._brush = fillColor.isValid() ? QBrush(fillColor) : Qt::NoBrush;

	if (ri.rule()._type == Rule::AnyType || ri.rule()._type == Rule::NodeType)
		_circles.append(ri);

	reader.skipCurrentElement();
}

void Style::text(QXmlStreamReader &reader, const MapData &data, const Rule &rule,
  QList<QList<TextRender>*> &lists)
{
	TextRender ri(rule);
	const QXmlStreamAttributes &attr = reader.attributes();
	int fontSize = 9;
	bool bold = false, italic = false;
	QString fontFamily("Helvetica");
	QFont::Capitalization capitalization = QFont::MixedCase;
	bool ok;

	if (attr.hasAttribute("k"))
		ri._key = data.tagId(attr.value("k").toLatin1());
	if (attr.hasAttribute("fill"))
		ri._fillColor = QColor(attr.value("fill").toString());
	if (attr.hasAttribute("stroke"))
		ri._strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("stroke-width")) {
		ri._strokeWidth = attr.value("stroke-width").toFloat(&ok);
		if (!ok || ri._strokeWidth < 0) {
			reader.raiseError("invalid stroke-width value");
			return;
		}
	}
	if (attr.hasAttribute("font-size")) {
		fontSize = attr.value("font-size").toFloat(&ok);
		if (!ok || fontSize < 0) {
			reader.raiseError("invalid font-size value");
			return;
		}
	}
	if (attr.hasAttribute("font-style")) {
		QString style(attr.value("font-style").toString());
		if (style == "bold")
			bold = true;
		else if (style == "italic")
			italic = true;
		else if (style == "bold_italic") {
			bold = true;
			italic = true;
		}
	}
	if (attr.hasAttribute("font-family")) {
		QString family(attr.value("font-family").toString());
		if (family == "monospace")
			fontFamily = "Courier New";
		else if (family == "serif")
			fontFamily = "Times New Roman";
	}
	if (attr.hasAttribute("text-transform")) {
		QString transform(attr.value("text-transform").toString());
		if (transform == "uppercase")
			capitalization = QFont::AllUppercase;
		else if (transform == "lowercase")
			capitalization = QFont::AllLowercase;
		else if (transform == "capitalize")
			capitalization = QFont::Capitalize;
	}
	if (attr.hasAttribute("priority")) {
		ri._priority = attr.value("priority").toInt(&ok);
		if (!ok) {
			reader.raiseError("invalid priority value");
			return;
		}
	}
	if (attr.hasAttribute("symbol-id"))
		ri._symbolId = attr.value("symbol-id").toString();

	ri._font.setFamily(fontFamily);
	ri._font.setPixelSize(fontSize);
	ri._font.setBold(bold);
	ri._font.setItalic(italic);
	ri._font.setCapitalization(capitalization);

	if (fontSize)
		for (int i = 0; i < lists.size(); i++)
			lists[i]->append(ri);

	reader.skipCurrentElement();
}

void Style::symbol(QXmlStreamReader &reader, const QString &dir, qreal ratio,
  const Rule &rule, QList<Symbol> &list)
{
	Symbol ri(rule);
	const QXmlStreamAttributes &attr = reader.attributes();
	QString file;
	int height = 0, width = 0, percent = 100;
	bool ok;

	if (attr.hasAttribute("src"))
		file = resourcePath(attr.value("src").toString(), dir);
	else {
		reader.raiseError("missing src value");
		return;
	}
	if (attr.hasAttribute("symbol-height")) {
		height = attr.value("symbol-height").toInt(&ok);
		if (!ok || height < 0) {
			reader.raiseError("invalid symbol-height value");
			return;
		}
	}
	if (attr.hasAttribute("symbol-width")) {
		width = attr.value("symbol-width").toInt(&ok);
		if (!ok || width < 0) {
			reader.raiseError("invalid symbol-width value");
			return;
		}
	}
	if (attr.hasAttribute("symbol-percent")) {
		percent = attr.value("symbol-percent").toInt(&ok);
		if (!ok || percent < 0) {
			reader.raiseError("invalid symbol-percent value");
			return;
		}
	}
	if (attr.hasAttribute("priority")) {
		ri._priority = attr.value("priority").toInt(&ok);
		if (!ok) {
			reader.raiseError("invalid priority value");
			return;
		}
	}
	if (attr.hasAttribute("rotate")) {
		if (attr.value("rotate").toString() == "false")
			ri._rotate = false;
	}
	if (attr.hasAttribute("id"))
		ri._id = attr.value("id").toString();

	ri._img = image(file, width, height, percent, ratio);

	list.append(ri);

	reader.skipCurrentElement();
}

void Style::rule(QXmlStreamReader &reader, const QString &dir,
  const MapData &data, qreal ratio, qreal baseStrokeWidth,
  const QSet<QString> &cats, const Rule &parent)
{
	Rule r(parent);
	const QXmlStreamAttributes &attr = reader.attributes();
	bool ok;

	if (attr.hasAttribute("cat")
	  && !cats.contains(attr.value("cat").toString())) {
		reader.skipCurrentElement();
		return;
	}

	if (attr.value("e").toString() == "way")
		r.setType(Rule::WayType);
	else if (attr.value("e").toString() == "node")
		r.setType(Rule::NodeType);

	if (attr.hasAttribute("zoom-min")) {
		r.setMinZoom(attr.value("zoom-min").toInt(&ok));
		if (!ok || r._zooms.min() < 0) {
			reader.raiseError("invalid zoom-min value");
			return;
		}
	}
	if (attr.hasAttribute("zoom-max")) {
		r.setMaxZoom(attr.value("zoom-max").toInt(&ok));
		if (!ok || r._zooms.max() < 0) {
			reader.raiseError("invalid zoom-max value");
			return;
		}
	}

	if (attr.hasAttribute("closed")) {
		if (attr.value("closed").toString() == "yes")
			r.setClosed(Rule::YesClosed);
		else if (attr.value("closed").toString() == "no")
			r.setClosed(Rule::NoClosed);
	}

	QList<QByteArray> keys(attr.value("k").toLatin1().split('|'));
	QList<QByteArray> vals(attr.value("v").toLatin1().split('|'));
	r.addFilter(Rule::Filter(data, keys, vals));

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("rule"))
			rule(reader, dir, data, ratio, baseStrokeWidth, cats, r);
		else if (reader.name() == QLatin1String("area"))
			area(reader, dir, ratio, baseStrokeWidth, r);
		else if (reader.name() == QLatin1String("line"))
			line(reader, baseStrokeWidth, r);
		else if (reader.name() == QLatin1String("circle"))
			circle(reader, baseStrokeWidth, r);
		else if (reader.name() == QLatin1String("pathText")) {
			QList<QList<TextRender>*> list;
			list.append(&_pathLabels);
			text(reader, data, r, list);
		} else if (reader.name() == QLatin1String("caption")) {
			QList<QList<TextRender>*> list;
			if (r._type == Rule::WayType || r._type == Rule::AnyType)
				list.append(&_areaLabels);
			if (r._type == Rule::NodeType || r._type == Rule::AnyType)
				list.append(&_pointLabels);
			text(reader, data, r, list);
		}
		else if (reader.name() == QLatin1String("symbol"))
			symbol(reader, dir, ratio, r, _symbols);
		else if (reader.name() == QLatin1String("lineSymbol"))
			symbol(reader, dir, ratio, r, _lineSymbols);
		else
			reader.skipCurrentElement();
	}
}

QString Style::cat(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();

	if (!attr.hasAttribute("id")) {
		reader.raiseError("Missing id attribute");
		return QString();
	}

	QString id(attr.value("id").toString());
	reader.skipCurrentElement();

	return id;
}

Style::Menu::Layer Style::layer(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	if (!attr.hasAttribute("id")) {
		reader.raiseError("Missing id attribute");
		return Menu::Layer();
	}

	Menu::Layer l(attr.value("id").toString(),
	  attr.value("enabled").toString() == "true");

	if (attr.hasAttribute("parent"))
		l.setParent(attr.value("parent").toString());

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("cat"))
			l.addCat(cat(reader));
		else if (reader.name() == QLatin1String("overlay"))
			l.addOverlay(cat(reader));
		else
			reader.skipCurrentElement();
	}

	return l;
}

Style::Menu Style::stylemenu(QXmlStreamReader &reader)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	if (!attr.hasAttribute("defaultvalue")) {
		reader.raiseError("Missing defaultvalue attribute");
		return Menu();
	}

	Style::Menu menu(attr.value("defaultvalue").toString());

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("layer"))
			menu.addLayer(layer(reader));
		else
			reader.skipCurrentElement();
	}

	return menu;
}

void Style::rendertheme(QXmlStreamReader &reader, const QString &dir,
  const MapData &data, qreal ratio)
{
	Rule r;
	QSet<QString> cats;
	qreal baseStrokeWidth = 1.0;

	const QXmlStreamAttributes &attr = reader.attributes();
	if (attr.hasAttribute("base-stroke-width")) {
		bool ok;
		baseStrokeWidth = attr.value("base-stroke-width").toFloat(&ok);
		if (!ok || baseStrokeWidth < 0) {
			reader.raiseError("invalid base-stroke-width value");
			return;
		}
	}

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("rule"))
			rule(reader, dir, data, ratio, baseStrokeWidth, cats, r);
		else if (reader.name() == QLatin1String("stylemenu")) {
			Menu menu(stylemenu(reader));
			cats = menu.cats();
		} else
			reader.skipCurrentElement();
	}
}

bool Style::loadXml(const QString &path, const MapData &data, qreal ratio)
{
	QFile file(path);
	if (!file.open(QFile::ReadOnly))
		return false;
	QXmlStreamReader reader(&file);
	QFileInfo fi(path);

	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("rendertheme"))
			rendertheme(reader, fi.absolutePath(), data, ratio);
		else
			reader.raiseError("Not a Mapsforge style file");
	}

	if (reader.error())
		qWarning("%s:%lld %s", qPrintable(path), reader.lineNumber(),
		  qPrintable(reader.errorString()));

	return !reader.error();
}

void Style::load(const MapData &data, qreal ratio)
{
	QString path(ProgramPaths::renderthemeFile());

	if (!QFileInfo::exists(path) || !loadXml(path, data, ratio))
		loadXml(":/mapsforge/default.xml", data, ratio);
}

void Style::clear()
{
	_paths.clear();
	_circles.clear();
	_pathLabels.clear();
	_pointLabels.clear();
	_areaLabels.clear();
	_symbols.clear();
}

QList<const Style::PathRender *> Style::paths(int zoom, bool closed,
  const QVector<MapData::Tag> &tags) const
{
	QList<const PathRender*> ri;

	for (int i = 0; i < _paths.size(); i++)
		if (_paths.at(i).rule().match(zoom, closed, tags))
			ri.append(&_paths.at(i));

	return ri;
}

QList<const Style::CircleRender *> Style::circles(int zoom,
  const QVector<MapData::Tag> &tags) const
{
	QList<const CircleRender*> ri;

	for (int i = 0; i < _circles.size(); i++)
		if (_circles.at(i).rule().match(zoom, tags))
			ri.append(&_circles.at(i));

	return ri;
}

QList<const Style::TextRender*> Style::pathLabels(int zoom) const
{
	QList<const TextRender*> list;

	for (int i = 0; i < _pathLabels.size(); i++)
		if (_pathLabels.at(i).rule()._zooms.contains(zoom))
			list.append(&_pathLabels.at(i));

	return list;
}

QList<const Style::TextRender*> Style::pointLabels(int zoom) const
{
	QList<const TextRender*> list;

	for (int i = 0; i < _pointLabels.size(); i++)
		if (_pointLabels.at(i).rule()._zooms.contains(zoom))
			list.append(&_pointLabels.at(i));

	return list;
}

QList<const Style::TextRender*> Style::areaLabels(int zoom) const
{
	QList<const TextRender*> list;

	for (int i = 0; i < _areaLabels.size(); i++)
		if (_areaLabels.at(i).rule()._zooms.contains(zoom))
			list.append(&_areaLabels.at(i));

	return list;
}

QList<const Style::Symbol*> Style::pointSymbols(int zoom) const
{
	QList<const Symbol*> list;

	for (int i = 0; i < _symbols.size(); i++) {
		const Symbol &symbol = _symbols.at(i);
		const Rule &rule = symbol.rule();
		if (rule._zooms.contains(zoom) && (rule._type == Rule::AnyType
		  || rule._type == Rule::NodeType))
			list.append(&symbol);
	}

	return list;
}

QList<const Style::Symbol*> Style::lineSymbols(int zoom) const
{
	QList<const Symbol*> list;

	for (int i = 0; i < _lineSymbols.size(); i++) {
		const Symbol &symbol = _lineSymbols.at(i);
		if (symbol.rule()._zooms.contains(zoom))
			list.append(&symbol);
	}

	return list;
}

QList<const Style::Symbol*> Style::areaSymbols(int zoom) const
{
	QList<const Symbol*> list;

	for (int i = 0; i < _symbols.size(); i++) {
		const Symbol &symbol = _symbols.at(i);
		const Rule &rule = symbol.rule();
		if (rule._zooms.contains(zoom) && (rule._type == Rule::AnyType
		  || rule._type == Rule::WayType))
			list.append(&symbol);
	}

	return list;
}

QPen Style::PathRender::pen(int zoom) const
{
	if (_strokeColor.isValid()) {
		qreal width = (_scale > None && zoom >= 12)
		  ? pow(1.5, zoom - 12) * _strokeWidth : _strokeWidth;
		QPen p(QBrush(_strokeColor), width, Qt::SolidLine, _strokeCap,
		  _strokeJoin);
		if (!_strokeDasharray.isEmpty()) {
			QVector<qreal>pattern(_strokeDasharray);
			for (int i = 0; i < _strokeDasharray.size(); i++) {
				if (_scale > Stroke && zoom >= 12)
					pattern[i] = (pow(1.5, zoom - 12) * pattern[i]);
				// QPainter pattern is specified in units of the pens width!
				pattern[i] /= width;
			}
			p.setDashPattern(pattern);
		}
		return p;
	} else
		return Qt::NoPen;
}

qreal Style::PathRender::dy(int zoom) const
{
	return (_scale && zoom >= 12) ? pow(1.5, zoom - 12) * _dy : _dy;
}

qreal Style::CircleRender::radius(int zoom) const
{
	return (_scale && zoom >= 12) ? pow(1.5, zoom - 12) * _radius : _radius;
}
