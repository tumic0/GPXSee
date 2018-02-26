#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <QTextStream>
#include <QStringList>
#include <QtAlgorithms>
#include "downloader.h"
#include "pcs.h"
#include "wmts.h"


Downloader *WMTS::_downloader = 0;

bool WMTS::createProjection(const QString &crs)
{
	QStringList list(crs.split(':'));
	QString authority, code;
	bool res;
	int epsg;
	const PCS *pcs;
	const GCS *gcs;

	switch (list.size()) {
		case 2:
			authority = list.at(0);
			code = list.at(1);
			break;
		case 7:
			authority = list.at(4);
			code = list.at(6);
			break;
		default:
			return false;
	}

	if (authority == "EPSG") {
		epsg = code.toInt(&res);
		if (!res)
			return false;

		if ((pcs = PCS::pcs(epsg))) {
			_projection = Projection(pcs->gcs(), pcs->method(), pcs->setup(),
			  pcs->units());
			return true;
		} else if ((gcs = GCS::gcs(epsg))) {
			_projection = Projection(gcs);
			return true;
		} else
			return false;
	} else if (authority == "OGC") {
		if (code == "CRS84") {
			_projection = Projection(GCS::gcs(4326));
			return true;
		} else
			return false;
	} else
		return false;
}

WMTS::TileMatrix WMTS::tileMatrix(QXmlStreamReader &reader, bool yx)
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
			if (yx)
				ts >> matrix.topLeft.ry() >> matrix.topLeft.rx();
			else
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

void WMTS::tileMatrixSet(CTX &ctx)
{
	QString id, crs;
	QSet<TileMatrix> matrixes;

	while (ctx.reader.readNextStartElement()) {
		if (ctx.reader.name() == "Identifier")
			id = ctx.reader.readElementText();
		else if (ctx.reader.name() == "SupportedCRS")
			crs = ctx.reader.readElementText();
		else if (ctx.reader.name() == "TileMatrix")
			matrixes.insert(tileMatrix(ctx.reader, ctx.setup.yx));
		else
			ctx.reader.skipCurrentElement();
	}

	if (id == ctx.setup.set) {
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

void WMTS::tileMatrixSetLink(CTX &ctx)
{
	QString id;
	QSet<MatrixLimits> limits;

	while (ctx.reader.readNextStartElement()) {
		if (ctx.reader.name() == "TileMatrixSet")
			id = ctx.reader.readElementText();
		else if (ctx.reader.name() == "TileMatrixSetLimits")
			limits = tileMatrixSetLimits(ctx.reader);
		else
			ctx.reader.skipCurrentElement();
	}

	if (id == ctx.setup.set) {
		ctx.set = true;
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

void WMTS::layer(CTX &ctx)
{
	QString id, tpl;
	RectC bounds;
	QStringList formats, styles;

	while (ctx.reader.readNextStartElement()) {
		if (ctx.reader.name() == "Identifier")
			id = ctx.reader.readElementText();
		else if (ctx.reader.name() == "TileMatrixSetLink")
			tileMatrixSetLink(ctx);
		else if (ctx.reader.name() == "WGS84BoundingBox")
			bounds = wgs84BoundingBox(ctx.reader);
		else if (ctx.reader.name() == "ResourceURL") {
			const QXmlStreamAttributes &attr = ctx.reader.attributes();
			if (attr.value("resourceType") == "tile")
				tpl = attr.value("template").toString();
			ctx.reader.skipCurrentElement();
		} else if (ctx.reader.name() == "Style")
			styles.append(style(ctx.reader));
		else if (ctx.reader.name() == "Format")
			formats.append(ctx.reader.readElementText());
		else
			ctx.reader.skipCurrentElement();
	}

	if (id == ctx.setup.layer) {
		ctx.layer = true;
		_bounds = bounds;
		if (ctx.setup.rest)
			_tileUrl = tpl;
		if (styles.contains(ctx.setup.style))
			ctx.style = true;
		if (formats.contains(ctx.setup.format))
			ctx.format = true;
	}
}

void WMTS::contents(CTX &ctx)
{
	while (ctx.reader.readNextStartElement()) {
		if (ctx.reader.name() == "TileMatrixSet")
			tileMatrixSet(ctx);
		else if (ctx.reader.name() == "Layer")
			layer(ctx);
		else
			ctx.reader.skipCurrentElement();
	}
}

void WMTS::capabilities(CTX &ctx)
{
	while (ctx.reader.readNextStartElement()) {
		if (ctx.reader.name() == "Contents")
			contents(ctx);
		else
			ctx.reader.skipCurrentElement();
	}
}

bool WMTS::parseCapabilities(const QString &path, const Setup &setup)
{
	QFile file(path);
	CTX ctx(setup);

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	ctx.reader.setDevice(&file);
	if (ctx.reader.readNextStartElement()) {
		if (ctx.reader.name() == "Capabilities")
			capabilities(ctx);
		else
			ctx.reader.raiseError("Not a Capabilities XML file");
	}
	if (ctx.reader.error()) {
		_errorString = QString("%1:%2: %3").arg(path).arg(
		  ctx.reader.lineNumber()).arg(ctx.reader.errorString());
		return false;
	}

	if (!ctx.layer) {
		_errorString = ctx.setup.layer + ": layer not provided";
		return false;
	}
	if (!ctx.style) {
		_errorString = ctx.setup.style + ": style not provided";
		return false;
	}
	if (!ctx.setup.rest && !ctx.format) {
		_errorString = ctx.setup.format + ": format not provided";
		return false;
	}
	if (!ctx.set) {
		_errorString = ctx.setup.set + ": set not provided";
		return false;
	}
	if (ctx.crs.isNull()) {
		_errorString = "Missing CRS definition";
		return false;
	}
	if (!createProjection(ctx.crs)) {
		_errorString = ctx.crs + ": unknown CRS";
		return false;
	}
	if (_matrixes.isEmpty()) {
		_errorString = "No usable tile matrix found";
		return false;
	}
	if (ctx.setup.rest && _tileUrl.isNull()) {
		_errorString = "Missing tile URL template";
		return false;
	}

	return true;
}

bool WMTS::getCapabilities(const QString &url, const QString &file)
{
	QList<Download> dl;

	dl.append(Download(url, file));

	QEventLoop wait;
	QObject::connect(_downloader, SIGNAL(finished()), &wait, SLOT(quit()));
	if (_downloader->get(dl))
		wait.exec();

	if (QFileInfo(file).exists())
		return true;
	else {
		_errorString = "Error downloading capabilities XML file";
		return false;
	}
}

bool WMTS::load(const QString &file, const WMTS::Setup &setup)
{
	QString capaUrl = setup.rest ? setup.url :
	  QString("%1?service=WMTS&Version=1.0.0&request=GetCapabilities")
	  .arg(setup.url);

	if (!QFileInfo(file).exists())
		if (!getCapabilities(capaUrl, file))
			return false;
	if (!parseCapabilities(file, setup))
		return false;

	if (!setup.rest)
		_tileUrl = QString("%1?service=WMTS&Version=1.0.0&request=GetTile"
		  "&Format=%2&Layer=%3&Style=%4&TileMatrixSet=%5&TileMatrix=$z"
		  "&TileRow=$y&TileCol=$x").arg(setup.url).arg(setup.format)
		  .arg(setup.layer).arg(setup.style).arg(setup.set);
	else {
		_tileUrl.replace("{Style}", setup.style);
		_tileUrl.replace("{TileMatrixSet}", setup.set);
		_tileUrl.replace("{TileMatrix}", "$z");
		_tileUrl.replace("{TileRow}", "$y");
		_tileUrl.replace("{TileCol}", "$x");
	}

	return true;
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
			zooms.append(Zoom(Zoom(mi->id, mi->scaleDenominator, mi->topLeft,
			  mi->tile, mi->matrix, li->rect)));
	}

	qSort(zooms);

	return zooms;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const WMTS::Setup &setup)
{
	dbg.nospace() << "Setup(" << setup.url << ", " << setup.layer << ", "
	  << setup.set << ", " << setup.style << ", " << setup.format << ", "
	  << setup.rest << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.id << ", " << zoom.scaleDenominator << ", "
	  << zoom.topLeft << ", " << zoom.tile << ", " << zoom.matrix << ", "
	  << zoom.limits << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
