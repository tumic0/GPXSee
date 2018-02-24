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

void WMTS::tileMatrix(QXmlStreamReader &reader)
{
	Zoom zoom;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			zoom.id = reader.readElementText();
		else if (reader.name() == "ScaleDenominator")
			zoom.scaleDenominator = reader.readElementText().toDouble();
		else if (reader.name() == "TopLeftCorner") {
			QString str = reader.readElementText();
			QTextStream(&str) >> zoom.topLeft.rx() >> zoom.topLeft.ry();
		} else if (reader.name() == "TileWidth")
			zoom.tile.setWidth(reader.readElementText().toInt());
		else if (reader.name() == "TileHeight")
			zoom.tile.setHeight(reader.readElementText().toInt());
		else if (reader.name() == "MatrixWidth")
			zoom.matrix.setWidth(reader.readElementText().toInt());
		else if (reader.name() == "MatrixHeight")
			zoom.matrix.setHeight(reader.readElementText().toInt());
		else
			reader.skipCurrentElement();
	}

	Zoom &z = _zooms[zoom.id];
	z.id = zoom.id;
	z.matrix = zoom.matrix;
	z.scaleDenominator = zoom.scaleDenominator;
	z.tile = zoom.tile;
	z.topLeft = zoom.topLeft;
}

void WMTS::tileMatrixSet(QXmlStreamReader &reader, const QString &set)
{
	QString id;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			id = reader.readElementText();
		else if (reader.name() == "SupportedCRS" && id == set) {
			if (!createProjection(reader.readElementText()))
				reader.raiseError("Invalid/unknown CRS");
		} else if (reader.name() == "TileMatrix" && id == set)
			tileMatrix(reader);
		else
			reader.skipCurrentElement();
	}
}

void WMTS::tileMatrixLimits(QXmlStreamReader &reader)
{
	QString id;
	QRect limits;

	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrix")
			id = reader.readElementText();
		else if (reader.name() == "MinTileRow")
			limits.setTop(reader.readElementText().toInt());
		else if (reader.name() == "MaxTileRow")
			limits.setBottom(reader.readElementText().toInt());
		else if (reader.name() == "MinTileCol")
			limits.setLeft(reader.readElementText().toInt());
		else if (reader.name() == "MaxTileCol")
			limits.setRight(reader.readElementText().toInt());
		else
			reader.skipCurrentElement();
	}

	_zooms[id].limits = limits;
}

void WMTS::tileMatrixSetLimits(QXmlStreamReader &reader)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixLimits")
			tileMatrixLimits(reader);
		else
			reader.skipCurrentElement();
	}
}

void WMTS::tileMatrixSetLink(QXmlStreamReader &reader, const QString &set)
{
	QString id;

	while (reader.readNextStartElement()) {
		if (reader.name() == "TileMatrixSet")
			id = reader.readElementText();
		else if (reader.name() == "TileMatrixSetLimits" && id == set)
			tileMatrixSetLimits(reader);
		else
			reader.skipCurrentElement();
	}
}

void WMTS::layer(QXmlStreamReader &reader, const QString &layer,
  const QString &set)
{
	QString id;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Identifier")
			id = reader.readElementText();
		else if (reader.name() == "TileMatrixSetLink" && id == layer)
			tileMatrixSetLink(reader, set);
		else
			reader.skipCurrentElement();
	}
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
	if (!QFileInfo(file).exists())
		if (!getCapabilities(url, file))
			return false;
	if (!parseCapabilities(file, layer, set))
		return false;

	if (_projection.isNull()) {
		_errorString = "Missing CRS definition";
		return false;
	}
	if (_zooms.isEmpty()) {
		_errorString = "No tile matrix found";
		return false;
	}

	return true;
}

QList<WMTS::Zoom> WMTS::zooms() const
{
	QList<Zoom> z(_zooms.values());
	qSort(z);
	return z;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.scaleDenominator << ", " << zoom.topLeft
	  << ", " << zoom.tile << ", " << zoom.matrix << ", " << zoom.limits << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
