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
			QTextStream(&str) >> matrix.topLeft.rx() >> matrix.topLeft.ry();
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

void WMTS::tileMatrixSet(QXmlStreamReader &reader, const QString &set)
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

	if (id == set) {
		if (!createProjection(crs)) {
			reader.raiseError("Invalid/unknown CRS");
			return;
		}

		_matrixes.unite(matrixes);
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

void WMTS::tileMatrixSetLink(QXmlStreamReader &reader, const QString &set)
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

	if (id == set)
		_limits.unite(limits);
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

void WMTS::layer(QXmlStreamReader &reader, const QString &layer,
  const QString &set)
{
	QString id;
	RectC bounds;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			id = reader.readElementText();
		else if (reader.name() == "TileMatrixSetLink")
			tileMatrixSetLink(reader, set);
		else if (reader.name() == "WGS84BoundingBox")
			bounds = wgs84BoundingBox(reader);
		else
			reader.skipCurrentElement();
	}

	if (id == layer)
		_bounds = bounds;
}

void WMTS::contents(QXmlStreamReader &reader, const QString &layer,
  const QString &set)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixSet")
			tileMatrixSet(reader, set);
		else if (reader.name() == "Layer")
			WMTS::layer(reader, layer, set);
		else
			reader.skipCurrentElement();
	}
}

void WMTS::capabilities(QXmlStreamReader &reader, const QString &layer,
  const QString &set)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Contents")
			contents(reader, layer, set);
		else
			reader.skipCurrentElement();
	}
}

bool WMTS::parseCapabilities(const QString &path, const QString &layer,
  const QString &set)
{
	QFile file(path);
	QXmlStreamReader reader;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	reader.setDevice(&file);

	if (reader.readNextStartElement()) {
		if (reader.name() == "Capabilities")
			capabilities(reader, layer, set);
		else
			reader.raiseError("Not a Capabilities XML file");
	}

	_errorString = reader.error() ? QString("%1:%2: %3").arg(path)
	  .arg(reader.lineNumber()).arg(reader.errorString()) : QString();

	return reader.error() ? false : true;
}

bool WMTS::getCapabilities(const QString &url, const QString &file)
{
	QList<Download> dl;

	QString capabilitiesUrl = QString("%1?service=WMTS&Version=1.0.0"
	  "&request=GetCapabilities").arg(url);
	dl.append(Download(capabilitiesUrl, file));

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

bool WMTS::load(const QString &file, const QString &url, const QString &layer,
  const QString &set)
{
	QMap<QString, Zoom>::const_iterator it;

	if (!QFileInfo(file).exists())
		if (!getCapabilities(url, file))
			return false;
	if (!parseCapabilities(file, layer, set))
		return false;

	if (_matrixes.isEmpty()) {
		_errorString = "No usable tile matrix found";
		return false;
	}
	if (_projection.isNull()) {
		_errorString = "Missing CRS definition";
		return false;
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
QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.id << ", " << zoom.scaleDenominator << ", "
	  << zoom.topLeft << ", " << zoom.tile << ", " << zoom.matrix << ", "
	  << zoom.limits << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
