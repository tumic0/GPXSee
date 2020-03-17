#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <QTextStream>
#include <QStringList>
#include <QtAlgorithms>
#include <QXmlStreamReader>
#include "downloader.h"
#include "pcs.h"
#include "crs.h"
#include "wmts.h"


static QString bareFormat(const QString &format)
{
	return format.left(format.indexOf(';')).trimmed();
}

static void skipParentElement(QXmlStreamReader &reader)
{
	while (reader.readNextStartElement())
		reader.skipCurrentElement();
}

WMTS::TileMatrix WMTS::tileMatrix(QXmlStreamReader &reader)
{
	TileMatrix matrix;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			matrix.id = reader.readElementText();
		else if (reader.name() == "ScaleDenominator")
			matrix.scaleDenominator = reader.readElementText().toDouble();
		else if (reader.name() == "TopLeftCorner") {
			QString str = reader.readElementText();
			QTextStream ts(&str);
			ts >> matrix.topLeft.rx() >> matrix.topLeft.ry();
		} else if (reader.name() == "TileWidth")
			matrix.tile.setWidth(reader.readElementText().toInt());
		else if (reader.name() == "TileHeight")
			matrix.tile.setHeight(reader.readElementText().toInt());
		else if (reader.name() == "MatrixWidth")
			matrix.matrix.setWidth(reader.readElementText().toInt());
		else if (reader.name() == "MatrixHeight")
			matrix.matrix.setHeight(reader.readElementText().toInt());
		else
			reader.skipCurrentElement();
	}

	if (!matrix.isValid())
		reader.raiseError("Invalid TileMatrix definition");

	return matrix;
}

void WMTS::tileMatrixSet(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier") {
			if (reader.readElementText() != _setup.set()) {
				skipParentElement(reader);
				return;
			}
		} else if (reader.name() == "SupportedCRS")
			ctx.crs = reader.readElementText();
		else if (reader.name() == "TileMatrix")
			ctx.matrixes.insert(tileMatrix(reader));
		else
			reader.skipCurrentElement();
	}
}

WMTS::MatrixLimits WMTS::tileMatrixLimits(QXmlStreamReader &reader)
{
	MatrixLimits limits;

	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrix")
			limits.id = reader.readElementText();
		else if (reader.name() == "MinTileRow")
			limits.rect.setTop(reader.readElementText().toInt());
		else if (reader.name() == "MaxTileRow")
			limits.rect.setBottom(reader.readElementText().toInt());
		else if (reader.name() == "MinTileCol")
			limits.rect.setLeft(reader.readElementText().toInt());
		else if (reader.name() == "MaxTileCol")
			limits.rect.setRight(reader.readElementText().toInt());
		else
			reader.skipCurrentElement();
	}

	if (!limits.isValid())
		reader.raiseError("Invalid TileMatrixLimits definition");

	return limits;
}

QSet<WMTS::MatrixLimits> WMTS::tileMatrixSetLimits(QXmlStreamReader &reader)
{
	QSet<MatrixLimits> limits;

	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixLimits")
			limits.insert(tileMatrixLimits(reader));
		else
			reader.skipCurrentElement();
	}

	return limits;
}

void WMTS::tileMatrixSetLink(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixSet") {
			if (reader.readElementText() == _setup.set())
				ctx.hasSet = true;
			else {
				skipParentElement(reader);
				return;
			}
		} else if (reader.name() == "TileMatrixSetLimits")
			ctx.limits = tileMatrixSetLimits(reader);
		else
			reader.skipCurrentElement();
	}
}

RectC WMTS::wgs84BoundingBox(QXmlStreamReader &reader)
{
	Coordinates topLeft, bottomRight;

	while (reader.readNextStartElement()) {
		if (reader.name() == "LowerCorner") {
			QString str = reader.readElementText();
			QTextStream(&str) >> topLeft.rlon() >> bottomRight.rlat();
		} else if (reader.name() == "UpperCorner") {
			QString str = reader.readElementText();
			QTextStream(&str) >> bottomRight.rlon() >> topLeft.rlat();
		} else
			reader.skipCurrentElement();
	}

	return RectC(topLeft, bottomRight);
}

QString WMTS::style(QXmlStreamReader &reader)
{
	QString id;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			id = reader.readElementText();
		else
			reader.skipCurrentElement();
	}

	return id;
}

void WMTS::layer(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier") {
			if (reader.readElementText() == _setup.layer())
				ctx.hasLayer = true;
			else {
				skipParentElement(reader);
				return;
			}
		} else if (reader.name() == "TileMatrixSetLink")
			tileMatrixSetLink(reader, ctx);
		else if (reader.name() == "WGS84BoundingBox")
			ctx.bbox = wgs84BoundingBox(reader);
		else if (reader.name() == "ResourceURL") {
			const QXmlStreamAttributes &attr = reader.attributes();
			if (attr.value("resourceType") == "tile" && _setup.rest())
				_tileUrl = attr.value("template").toString();
			reader.skipCurrentElement();
		} else if (reader.name() == "Style") {
			const QXmlStreamAttributes &attr = reader.attributes();
			bool isDefault = (attr.value("isDefault") == "true");
			QString s = style(reader);
			if (isDefault)
				ctx.defaultStyle = s;
			if (s == _setup.style())
				ctx.hasStyle = true;
		} else if (reader.name() == "Format") {
			QString format(reader.readElementText());
			if (bareFormat(format) == bareFormat(_setup.format()))
				ctx.hasFormat = true;
		} else
			reader.skipCurrentElement();
	}
}

void WMTS::contents(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixSet")
			tileMatrixSet(reader, ctx);
		else if (reader.name() == "Layer")
			layer(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMTS::capabilities(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Contents")
			contents(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMTS::createZooms(const CTX &ctx)
{
	for (QSet<TileMatrix>::const_iterator mi = ctx.matrixes.constBegin();
	  mi != ctx.matrixes.constEnd(); ++mi) {
		QSet<MatrixLimits>::const_iterator li = ctx.limits.find(
		  MatrixLimits(mi->id));
		if (!ctx.limits.isEmpty() && li == ctx.limits.constEnd())
			continue;
		_zooms.append(Zoom(mi->id, mi->scaleDenominator, mi->topLeft, mi->tile,
		  mi->matrix, li == ctx.limits.constEnd() ? QRect() : li->rect));
	}

	qSort(_zooms);
}

bool WMTS::parseCapabilities(CTX &ctx)
{
	QFile file(_path);
	QXmlStreamReader reader;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	reader.setDevice(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == "Capabilities")
			capabilities(reader, ctx);
		else
			reader.raiseError("Not a Capabilities XML file");
	}
	if (reader.error()) {
		_errorString = QString("%1:%2: %3").arg(_path).arg(reader.lineNumber())
		  .arg(reader.errorString());
		return false;
	}

	if (!ctx.hasLayer) {
		_errorString = _setup.layer() + ": layer not provided";
		return false;
	}
	if (!ctx.hasStyle && !_setup.style().isEmpty()) {
		_errorString = _setup.style() + ": style not provided";
		return false;
	}
	if (!ctx.hasStyle && _setup.style().isEmpty()
	  && ctx.defaultStyle.isEmpty()) {
		_errorString = "Default style not provided";
		return false;
	}
	if (!_setup.rest() && !ctx.hasFormat) {
		_errorString = _setup.format() + ": format not provided";
		return false;
	}
	if (!ctx.hasSet) {
		_errorString = _setup.set() + ": set not provided";
		return false;
	}
	if (ctx.crs.isNull()) {
		_errorString = "Missing CRS definition";
		return false;
	}
	_projection = CRS::projection(ctx.crs);
	if (!_projection.isValid()) {
		_errorString = ctx.crs + ": unknown CRS";
		return false;
	}
	createZooms(ctx);
	if (_zooms.isEmpty()) {
		_errorString = "No usable tile matrix found";
		return false;
	}
	if (_setup.rest() && _tileUrl.isNull()) {
		_errorString = "Missing tile URL template";
		return false;
	}
	_bbox = ctx.bbox;
	_cs = (_setup.coordinateSystem().axisOrder() == CoordinateSystem::Unknown)
	  ? _projection.coordinateSystem() : _setup.coordinateSystem();

	return true;
}

bool WMTS::downloadCapabilities(const QString &url)
{
	if (!_downloader) {
		_downloader = new Downloader(this);
		connect(_downloader, SIGNAL(finished()), this,
		  SLOT(capabilitiesReady()));
	}

	QList<Download> dl;
	dl.append(Download(url, _path));

	return _downloader->get(dl, _setup.authorization());
}

void WMTS::capabilitiesReady()
{
	if (!QFileInfo(_path).exists()) {
		_errorString = "Error downloading capabilities XML file";
		_valid = false;
	} else {
		_ready = true;
		_valid = init();
	}

	emit downloadFinished();
}

bool WMTS::init()
{
	CTX ctx;
	if (!parseCapabilities(ctx))
		return false;

	QString style = _setup.style().isEmpty() ? ctx.defaultStyle : _setup.style();
	if (!_setup.rest()) {
		_tileUrl = QString("%1%2service=WMTS&Version=1.0.0&request=GetTile"
		  "&Format=%3&Layer=%4&Style=%5&TileMatrixSet=%6&TileMatrix=$z"
		  "&TileRow=$y&TileCol=$x").arg(_setup.url(),
		  _setup.url().contains('?') ? "&" : "?" , _setup.format(),
		  _setup.layer(), style, _setup.set());
		for (int i = 0; i < _setup.dimensions().size(); i++) {
			const KV<QString, QString> &dim = _setup.dimensions().at(i);
			_tileUrl.append(QString("&%1=%2").arg(dim.key(), dim.value()));
		}
	} else {
		_tileUrl.replace("{Style}", style, Qt::CaseInsensitive);
		_tileUrl.replace("{TileMatrixSet}", _setup.set(), Qt::CaseInsensitive);
		_tileUrl.replace("{TileMatrix}", "$z", Qt::CaseInsensitive);
		_tileUrl.replace("{TileRow}", "$y", Qt::CaseInsensitive);
		_tileUrl.replace("{TileCol}", "$x", Qt::CaseInsensitive);
		for (int i = 0; i < _setup.dimensions().size(); i++) {
			const KV<QString, QString> &dim = _setup.dimensions().at(i);
			_tileUrl.replace(QString("{%1}").arg(dim.key()), dim.value(),
			  Qt::CaseInsensitive);
		}
	}

	return true;
}

WMTS::WMTS(const QString &file, const WMTS::Setup &setup, QObject *parent)
  : QObject(parent), _setup(setup), _downloader(0), _valid(false), _ready(false)
{
	QUrl url(setup.rest() ? setup.url() : QString(
	  "%1%2service=WMTS&Version=1.0.0&request=GetCapabilities").arg(setup.url(),
	  setup.url().contains('?') ? "&" : "?"));

	_path = url.isLocalFile() ? url.toLocalFile() : file;

	if (!url.isLocalFile() && !QFileInfo(file).exists())
		_valid = downloadCapabilities(url.toString());
	else {
		_ready = true;
		_valid = init();
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const WMTS::Setup &setup)
{
	dbg.nospace() << "Setup(" << setup.url() << ", " << setup.layer() << ", "
	  << setup.set() << ", " << setup.style() << ", " << setup.format() << ", "
	  << setup.rest() << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.id() << ", " << zoom.scaleDenominator()
	  << ", " << zoom.topLeft() << ", " << zoom.tile() << ", " << zoom.matrix()
	  << ", " << zoom.limits() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
