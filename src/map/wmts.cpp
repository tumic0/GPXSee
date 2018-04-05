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


Downloader *WMTS::_downloader = 0;

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
	QString id, crs;
	QSet<TileMatrix> matrixes;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			id = reader.readElementText();
		else if (reader.name() == "SupportedCRS")
			crs = reader.readElementText();
		else if (reader.name() == "TileMatrix")
			matrixes.insert(tileMatrix(reader));
		else
			reader.skipCurrentElement();
	}

	if (id == ctx.setup.set()) {
		ctx.crs = crs;
		_matrixes = matrixes;
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
	QString id;
	QSet<MatrixLimits> limits;

	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixSet")
			id = reader.readElementText();
		else if (reader.name() == "TileMatrixSetLimits")
			limits = tileMatrixSetLimits(reader);
		else
			reader.skipCurrentElement();
	}

	if (id == ctx.setup.set()) {
		ctx.hasSet = true;
		_limits = limits;
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
	QString id, tpl;
	RectC bounds;
	QStringList formats, styles;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			id = reader.readElementText();
		else if (reader.name() == "TileMatrixSetLink")
			tileMatrixSetLink(reader, ctx);
		else if (reader.name() == "WGS84BoundingBox")
			bounds = wgs84BoundingBox(reader);
		else if (reader.name() == "ResourceURL") {
			const QXmlStreamAttributes &attr = reader.attributes();
			if (attr.value("resourceType") == "tile")
				tpl = attr.value("template").toString();
			reader.skipCurrentElement();
		} else if (reader.name() == "Style")
			styles.append(style(reader));
		else if (reader.name() == "Format")
			formats.append(reader.readElementText());
		else
			reader.skipCurrentElement();
	}

	if (id == ctx.setup.layer()) {
		ctx.hasLayer = true;
		_bounds = bounds;
		if (ctx.setup.rest())
			_tileUrl = tpl;
		if (styles.contains(ctx.setup.style()) || ctx.setup.style().isEmpty())
			ctx.hasStyle = true;
		if (formats.contains(ctx.setup.format()))
			ctx.hasFormat = true;
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

bool WMTS::parseCapabilities(const QString &path, const Setup &setup)
{
	QFile file(path);
	CTX ctx(setup);
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
		_errorString = QString("%1:%2: %3").arg(path).arg(reader.lineNumber())
		  .arg(reader.errorString());
		return false;
	}

	if (!ctx.hasLayer) {
		_errorString = ctx.setup.layer() + ": layer not provided";
		return false;
	}
	if (!ctx.hasStyle) {
		_errorString = ctx.setup.style() + ": style not provided";
		return false;
	}
	if (!ctx.setup.rest() && !ctx.hasFormat) {
		_errorString = ctx.setup.format() + ": format not provided";
		return false;
	}
	if (!ctx.hasSet) {
		_errorString = ctx.setup.set() + ": set not provided";
		return false;
	}
	if (ctx.crs.isNull()) {
		_errorString = "Missing CRS definition";
		return false;
	}
	_projection = CRS::projection(ctx.crs);
	if (_projection.isNull()) {
		_errorString = ctx.crs + ": unknown CRS";
		return false;
	}
	if (_matrixes.isEmpty()) {
		_errorString = "No usable tile matrix found";
		return false;
	}
	if (ctx.setup.rest() && _tileUrl.isNull()) {
		_errorString = "Missing tile URL template";
		return false;
	}

	return true;
}

bool WMTS::getCapabilities(const QString &url, const QString &file,
  const Authorization &authorization)
{
	QList<Download> dl;

	dl.append(Download(url, file));

	QEventLoop wait;
	QObject::connect(_downloader, SIGNAL(finished()), &wait, SLOT(quit()));
	if (_downloader->get(dl, authorization))
		wait.exec();

	if (QFileInfo(file).exists())
		return true;
	else {
		_errorString = "Error downloading capabilities XML file";
		return false;
	}
}

WMTS::WMTS(const QString &file, const WMTS::Setup &setup) : _valid(false)
{
	QString capaUrl = setup.rest() ? setup.url() :
	  QString("%1?service=WMTS&Version=1.0.0&request=GetCapabilities")
	  .arg(setup.url());

	if (!QFileInfo(file).exists())
		if (!getCapabilities(capaUrl, file, setup.authorization()))
			return;
	if (!parseCapabilities(file, setup))
		return;

	QString style = setup.style().isEmpty() ? "default" : setup.style();
	if (!setup.rest()) {
		_tileUrl = QString("%1?service=WMTS&Version=1.0.0&request=GetTile"
		  "&Format=%2&Layer=%3&Style=%4&TileMatrixSet=%5&TileMatrix=$z"
		  "&TileRow=$y&TileCol=$x").arg(setup.url(), setup.format(),
		  setup.layer(), style, setup.set());
		for (int i = 0; i < setup.dimensions().size(); i++) {
			const QPair<QString, QString> &dim = setup.dimensions().at(i);
			_tileUrl.append(QString("&%1=%2").arg(dim.first, dim.second));
		}
	} else {
		_tileUrl.replace("{Style}", style, Qt::CaseInsensitive);
		_tileUrl.replace("{TileMatrixSet}", setup.set(), Qt::CaseInsensitive);
		_tileUrl.replace("{TileMatrix}", "$z", Qt::CaseInsensitive);
		_tileUrl.replace("{TileRow}", "$y", Qt::CaseInsensitive);
		_tileUrl.replace("{TileCol}", "$x", Qt::CaseInsensitive);
		for (int i = 0; i < setup.dimensions().size(); i++) {
			const QPair<QString, QString> &dim = setup.dimensions().at(i);
			_tileUrl.replace(QString("{%1}").arg(dim.first), dim.second,
			  Qt::CaseInsensitive);
		}
	}

	_valid = true;
}

QList<WMTS::Zoom> WMTS::zooms() const
{
	QList<Zoom> zooms;
	QSet<TileMatrix>::const_iterator mi;
	QSet<MatrixLimits>::const_iterator li;

	for (mi = _matrixes.constBegin(); mi != _matrixes.constEnd(); ++mi) {
		if ((li = _limits.find(MatrixLimits(mi->id))) == _limits.constEnd())
			zooms.append(Zoom(mi->id, mi->scaleDenominator, mi->topLeft,
			  mi->tile, mi->matrix, QRect()));
		else
			zooms.append(Zoom(mi->id, mi->scaleDenominator, mi->topLeft,
			  mi->tile, mi->matrix, li->rect));
	}

	qSort(zooms);

	return zooms;
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
