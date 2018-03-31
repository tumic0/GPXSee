#include <QFileInfo>
#include <QEventLoop>
#include <QXmlStreamReader>
#include "downloader.h"
#include "crs.h"
#include "wms.h"


Downloader *WMS::_downloader = 0;

void WMS::getMap(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Format") {
			if (reader.readElementText() == ctx.setup.format())
				ctx.format = true;
		} else
			reader.skipCurrentElement();
	}
}

void WMS::request(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "GetMap")
			getMap(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

QString WMS::style(QXmlStreamReader &reader)
{
	QString name;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Name")
			name = reader.readElementText();
		else
			reader.skipCurrentElement();
	}

	return name;
}

void WMS::layer(QXmlStreamReader &reader, CTX &ctx,
  const QList<QString> &pCRSs, const QList<QString> &pStyles)
{
	QString name;
	QList<QString> CRSs(pCRSs);
	QList<QString> styles(pStyles);
	RangeF scaleDenominator(2132.729583849784, 559082264.0287178);
	QRectF boundingBox;


	while (reader.readNextStartElement()) {
		if (reader.name() == "Name")
			name = reader.readElementText();
		else if (reader.name() == "CRS")
			CRSs.append(reader.readElementText());
		else if (reader.name() == "Style")
			styles.append(style(reader));
		else if (reader.name() == "MinScaleDenominator")
			scaleDenominator.setMin(reader.readElementText().toDouble());
		else if (reader.name() == "MaxScaleDenominator")
			scaleDenominator.setMax(reader.readElementText().toDouble());
		else if (reader.name() == "BoundingBox") {
			QXmlStreamAttributes attr = reader.attributes();
			if (attr.value("CRS") == ctx.setup.crs()) {
				boundingBox = QRectF(QPointF(
				  attr.value("minx").toString().toDouble(),
				  attr.value("maxy").toString().toDouble()),
				  QPointF(attr.value("maxx").toString().toDouble(),
				  attr.value("miny").toString().toDouble()));
			}
			reader.skipCurrentElement();
		} else if (reader.name() == "Layer")
			layer(reader, ctx, CRSs, styles);
		else
			reader.skipCurrentElement();
	}

	if (name == ctx.setup.layer()) {
		ctx.scaleDenominator = scaleDenominator;
		ctx.boundingBox = boundingBox;
		ctx.layer = true;
		ctx.style = styles.contains(ctx.setup.style());
		ctx.crs = CRSs.contains(ctx.setup.crs());
	}
}

void WMS::capability(QXmlStreamReader &reader, CTX &ctx)
{
	QList<QString> CRSs;
	QList<QString> styles;

	while (reader.readNextStartElement()) {
		if (reader.name() == "Layer")
			layer(reader, ctx, CRSs, styles);
		else if (reader.name() == "Request")
			request(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMS::capabilities(QXmlStreamReader &reader, CTX &ctx)
{
	_version = reader.attributes().value("version").toString();

	while (reader.readNextStartElement()) {
		if (reader.name() == "Capability")
			capability(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

bool WMS::parseCapabilities(const QString &path, const Setup &setup)
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
		if (reader.name() == "WMS_Capabilities")
			capabilities(reader, ctx);
		else
			reader.raiseError("Not a WMS_Capabilities XML file");
	}
	if (reader.error()) {
		_errorString = QString("%1:%2: %3").arg(path).arg(reader.lineNumber())
		  .arg(reader.errorString());
		return false;
	}

	if (!ctx.layer) {
		_errorString = ctx.setup.layer() + ": layer not provided";
		return false;
	}
	if (!ctx.style) {
		_errorString = ctx.setup.style() + ": style not provided";
		return false;
	}
	if (!ctx.format) {
		_errorString = ctx.setup.format() + ": format not provided";
		return false;
	}
	if (!ctx.crs) {
		_errorString = ctx.setup.crs() + ": CRS not provided";
		return false;
	}
	_projection = CRS::projection(ctx.setup.crs());
	if (_projection.isNull()) {
		_errorString = ctx.setup.crs() + ": unknown CRS";
		return false;
	}
	if (!ctx.scaleDenominator.isValid()) {
		_errorString = "Invalid scale denominator range";
		return false;
	}
	if (ctx.boundingBox.isNull()) {
		_errorString = "Missing bounding box";
		return false;
	}

	_boundingBox = ctx.boundingBox;
	_scaleDenominator = ctx.scaleDenominator;

	return true;
}

bool WMS::getCapabilities(const QString &url, const QString &file)
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

WMS::WMS(const QString &file, const WMS::Setup &setup) : _valid(false)
{
	QString capaUrl = QString("%1?service=WMS&request=GetCapabilities")
	  .arg(setup.url());

	if (!QFileInfo(file).exists())
		if (!getCapabilities(capaUrl, file))
			return;
	if (!parseCapabilities(file, setup))
		return;

	_valid = true;
}
