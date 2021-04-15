#include <QFile>
#include <QXmlStreamReader>
#include <QUrl>
#include <QFileInfo>
#include <QImageReader>
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

static QImage image(const QString &path, int width, int height)
{
	QImageReader ir(path, "svg");

	if (ir.canRead()) {
		if (!height && !width) {
			height = 20;
			width = 20;
		} else if (!width) {
			width = ir.size().height() / (ir.size().height() / (double)height);
		} else if (!height)
			height = ir.size().width() / (ir.size().width() / (double)width);

		ir.setScaledSize(QSize(width, height));
		return ir.read();
	} else
		return QImage(path);
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

	if (_type && WayType != _type)
		return false;
	if (!_zooms.contains(zoom))
		return false;
	if (_closed && cl != _closed)
		return false;

	for (int i = 0; i < _filters.size(); i++)
		if (!_filters.at(i).match(tags))
			return false;

	return true;
}

void Style::area(QXmlStreamReader &reader, const QString &dir, const Rule &rule)
{
	PathRender ri(rule, _paths.size());
	const QXmlStreamAttributes &attr = reader.attributes();
	QString file;
	int height = 0, width = 0;

	if (attr.hasAttribute("fill"))
		ri._fillColor = QColor(attr.value("fill").toString());
	if (attr.hasAttribute("stroke"))
		ri._strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("stroke-width"))
		ri._strokeWidth = attr.value("stroke-width").toFloat();
	if (attr.hasAttribute("src"))
		file = resourcePath(attr.value("src").toString(), dir);
	if (attr.hasAttribute("symbol-height"))
		height = attr.value("symbol-height").toInt();
	if (attr.hasAttribute("symbol-width"))
		width = attr.value("symbol-width").toInt();

	if (!file.isNull())
		ri._fillImage = image(file, width, height);

	_paths.append(ri);

	reader.skipCurrentElement();
}

void Style::line(QXmlStreamReader &reader, const Rule &rule)
{
	PathRender ri(rule, _paths.size());
	const QXmlStreamAttributes &attr = reader.attributes();

	if (attr.hasAttribute("stroke"))
		ri._strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("stroke-width"))
		ri._strokeWidth = attr.value("stroke-width").toFloat();
	if (attr.hasAttribute("stroke-dasharray")) {
		QStringList l(attr.value("stroke-dasharray").toString().split(','));
		ri._strokeDasharray.resize(l.size());
		for (int i = 0; i < l.size(); i++)
			ri._strokeDasharray[i] = l.at(i).toDouble();
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

	_paths.append(ri);

	reader.skipCurrentElement();
}

void Style::text(QXmlStreamReader &reader, const Rule &rule,
  QList<QList<TextRender>*> &lists)
{
	TextRender ri(rule);
	const QXmlStreamAttributes &attr = reader.attributes();
	int fontSize = 9;
	bool bold = false, italic = false;

	if (attr.hasAttribute("k"))
		ri._key = attr.value("k").toLatin1();
	if (attr.hasAttribute("fill"))
		ri._fillColor = QColor(attr.value("fill").toString());
	if (attr.hasAttribute("stroke"))
		ri._strokeColor = QColor(attr.value("stroke").toString());
	if (attr.hasAttribute("font-size"))
		fontSize = attr.value("font-size").toInt();
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
	ri._font.setPixelSize(fontSize);
	ri._font.setBold(bold);
	ri._font.setItalic(italic);

	for (int i = 0; i < lists.size(); i++)
		lists[i]->append(ri);

	reader.skipCurrentElement();
}

void Style::symbol(QXmlStreamReader &reader, const QString &dir,
  const Rule &rule)
{
	Symbol ri(rule);
	const QXmlStreamAttributes &attr = reader.attributes();
	QString file;
	int height = 0, width = 0;

	if (attr.hasAttribute("src"))
		file = resourcePath(attr.value("src").toString(), dir);
	if (attr.hasAttribute("symbol-height"))
		height = attr.value("symbol-height").toInt();
	if (attr.hasAttribute("symbol-width"))
		width = attr.value("symbol-width").toInt();

	if (!file.isNull())
		ri._img = image(file, width, height);

	_symbols.append(ri);

	reader.skipCurrentElement();
}

void Style::rule(QXmlStreamReader &reader, const QString &dir,
  const QSet<QString> &cats, const Rule &parent)
{
	Rule r(parent);
	const QXmlStreamAttributes &attr = reader.attributes();

	if (attr.hasAttribute("cat")
	  && !cats.contains(attr.value("cat").toString())) {
		reader.skipCurrentElement();
		return;
	}

	if (attr.value("e").toString() == "way")
		r.setType(Rule::WayType);
	else if (attr.value("e").toString() == "node")
		r.setType(Rule::NodeType);

	if (attr.hasAttribute("zoom-min"))
		r.setMinZoom(attr.value("zoom-min").toInt());
	if (attr.hasAttribute("zoom-max"))
		r.setMaxZoom(attr.value("zoom-max").toInt());

	if (attr.hasAttribute("closed")) {
		if (attr.value("closed").toString() == "yes")
			r.setClosed(Rule::YesClosed);
		else if (attr.value("closed").toString() == "no")
			r.setClosed(Rule::NoClosed);
	}

	QList<QByteArray> keys(attr.value("k").toLatin1().split('|'));
	QList<QByteArray> vals(attr.value("v").toLatin1().split('|'));
	r.addFilter(Rule::Filter(keys, vals));

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("rule"))
			rule(reader, dir, cats, r);
		else if (reader.name() == QLatin1String("area"))
			area(reader, dir, r);
		else if (reader.name() == QLatin1String("line"))
			line(reader, r);
		else if (reader.name() == QLatin1String("pathText")) {
			QList<QList<TextRender>*> list;
			list.append(&_pathLabels);
			text(reader, r, list);
		} else if (reader.name() == QLatin1String("caption")) {
			QList<QList<TextRender>*> list;
			if (r._type == Rule::WayType || r._type == Rule::AnyType)
				list.append(&_areaLabels);
			if (r._type == Rule::NodeType || r._type == Rule::AnyType)
				list.append(&_pointLabels);
			text(reader, r, list);
		}
		else if (reader.name() == QLatin1String("symbol"))
			symbol(reader, dir, r);
		else
			reader.skipCurrentElement();
	}
}

void Style::cat(QXmlStreamReader &reader, QSet<QString> &cats)
{
	const QXmlStreamAttributes &attr = reader.attributes();

	cats.insert(attr.value("id").toString());

	reader.skipCurrentElement();
}

void Style::layer(QXmlStreamReader &reader, QSet<QString> &cats)
{
	const QXmlStreamAttributes &attr = reader.attributes();
	bool enabled = (attr.value("enabled").toString() == "true");

	while (reader.readNextStartElement()) {
		if (enabled && reader.name() == QLatin1String("cat"))
			cat(reader, cats);
		else
			reader.skipCurrentElement();
	}
}

void Style::stylemenu(QXmlStreamReader &reader, QSet<QString> &cats)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("layer"))
			layer(reader, cats);
		else
			reader.skipCurrentElement();
	}
}

void Style::rendertheme(QXmlStreamReader &reader, const QString &dir)
{
	Rule r;
	QSet<QString> cats;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("rule"))
			rule(reader, dir, cats, r);
		else if (reader.name() == QLatin1String("stylemenu"))
			stylemenu(reader, cats);
		else
			reader.skipCurrentElement();
	}
}

bool Style::loadXml(const QString &path)
{
	QFile file(path);
	if (!file.open(QFile::ReadOnly))
		return false;
	QXmlStreamReader reader(&file);
	QFileInfo fi(path);

	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("rendertheme"))
			rendertheme(reader, fi.absolutePath());
		else
			reader.raiseError("Not a Mapsforge style file");
	}

	if (reader.error())
		qWarning("%s:%lld %s", qPrintable(path), reader.lineNumber(),
		  qPrintable(reader.errorString()));

	return !reader.error();
}

Style::Style(const QString &path)
{
	if (!QFileInfo::exists(path) || !loadXml(path))
		loadXml(":/mapsforge/default.xml");
}

QVector<const Style::PathRender *> Style::paths(int zoom, bool closed,
  const QVector<MapData::Tag> &tags) const
{
	QVector<const Style::PathRender*> ri;

	for (int i = 0; i < _paths.size(); i++)
		if (_paths.at(i).rule().match(zoom, closed, tags))
			ri.append(&_paths.at(i));

	return ri;
}

QList<const Style::TextRender*> Style::pathLabels(int zoom) const
{
	QList<const Style::TextRender*> list;

	for (int i = 0; i < _pathLabels.size(); i++)
		if (_pathLabels.at(i).rule().zooms().contains(zoom))
			list.append(&_pathLabels.at(i));

	return list;
}

QList<const Style::TextRender*> Style::pointLabels(int zoom) const
{
	QList<const Style::TextRender*> list;

	for (int i = 0; i < _pointLabels.size(); i++)
		if (_pointLabels.at(i).rule().zooms().contains(zoom))
			list.append(&_pointLabels.at(i));

	return list;
}

QList<const Style::TextRender*> Style::areaLabels(int zoom) const
{
	QList<const Style::TextRender*> list;

	for (int i = 0; i < _areaLabels.size(); i++)
		if (_areaLabels.at(i).rule().zooms().contains(zoom))
			list.append(&_areaLabels.at(i));

	return list;
}

QList<const Style::Symbol*> Style::symbols(int zoom) const
{
	QList<const Style::Symbol*> list;

	for (int i = 0; i < _symbols.size(); i++)
		if (_symbols.at(i).rule().zooms().contains(zoom))
			list.append(&_symbols.at(i));

	return list;
}

QPen Style::PathRender::pen(int zoom) const
{
	qreal width = (zoom >= 12)
	  ? pow(1.5, zoom - 12) * _strokeWidth : _strokeWidth;

	if (_strokeColor.isValid()) {
		QPen p(QBrush(_strokeColor), width, Qt::SolidLine, _strokeCap,
		  _strokeJoin);
		if (!_strokeDasharray.isEmpty()) {
			QVector<qreal>pattern(_strokeDasharray);
			for (int i = 0; i < _strokeDasharray.size(); i++)
				pattern[i] /= width;
			p.setDashPattern(pattern);
		}
		return p;
	} else
		return Qt::NoPen;
}

QBrush Style::PathRender::brush() const
{
	if (!_fillImage.isNull())
		return QBrush(_fillImage);
	else if (_fillColor.isValid())
		return QBrush(_fillColor);
	else
		return Qt::NoBrush;
}
