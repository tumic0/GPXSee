#include <cmath>
#include <QFileInfo>
#include <QEventLoop>
#include <QXmlStreamReader>
#include <QStringList>
#include "downloader.h"
#include "crs.h"
#include "wms.h"


static QString bareFormat(const QString &format)
{
	return format.left(format.indexOf(';')).trimmed();
}

static inline double hint2denominator(double h)
{
	/* Some WMS 1.1.1 servers use a 72dpi resolution by default. Using the usual
	   90dpi (0.28mm) resolution known from later standards (WMS 1.3, WMTS) does
	   make them return emty images in the "max" scale level. */
	return h / (M_SQRT2 * 0.36e-3);
}

WMS::CTX::CTX(const Setup &setup) : setup(setup), formatSupported(false)
{
	QStringList ll = setup.layer().split(',');

	if (setup.style().isEmpty()) {
		for (int i = 0; i < ll.size(); i++)
			layers.append(Layer(ll.at(i)));
	} else {
		QStringList sl = setup.style().split(',');
		if (ll.size() != sl.size())
			return;

		for (int i = 0; i < ll.size(); i++)
			layers.append(Layer(ll.at(i), sl.at(i)));
	}
}

void WMS::get(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("OnlineResource")) {
			QXmlStreamAttributes attr = reader.attributes();
			ctx.url =  attr.value("xlink:href").toString();
			reader.skipCurrentElement();
		} else
			reader.skipCurrentElement();
	}
}

void WMS::http(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Get"))
			get(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMS::dcpType(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("HTTP"))
			http(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMS::getMap(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Format")) {
			QString format(reader.readElementText());
			if (bareFormat(format) == bareFormat(ctx.setup.format()))
				ctx.formatSupported = true;
		} else if (reader.name() == QLatin1String("DCPType"))
			dcpType(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMS::request(QXmlStreamReader &reader, CTX &ctx)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("GetMap"))
			getMap(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

QString WMS::style(QXmlStreamReader &reader)
{
	QString name;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Name"))
			name = reader.readElementText();
		else
			reader.skipCurrentElement();
	}

	return name;
}

RectC WMS::geographicBoundingBox(QXmlStreamReader &reader)
{
	double left = NAN, top = NAN, right = NAN, bottom = NAN;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("westBoundLongitude"))
			left = reader.readElementText().toDouble();
		else if (reader.name() == QLatin1String("eastBoundLongitude"))
			right = reader.readElementText().toDouble();
		else if (reader.name() == QLatin1String("northBoundLatitude"))
			top = reader.readElementText().toDouble();
		else if (reader.name() == QLatin1String("southBoundLatitude"))
			bottom = reader.readElementText().toDouble();
		else
			reader.skipCurrentElement();
	}

	return RectC(Coordinates(left, top), Coordinates(right, bottom));
}

void WMS::layer(QXmlStreamReader &reader, CTX &ctx,
  const QList<QString> &pCRSs, const QList<QString> &pStyles,
  RangeF &pScaleDenominator, RectC &pBoundingBox)
{
	QString name;
	QList<QString> CRSs(pCRSs);
	QList<QString> styles(pStyles);
	RangeF scaleDenominator(pScaleDenominator);
	RectC boundingBox(pBoundingBox);
	int index;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Name"))
			name = reader.readElementText();
		else if (reader.name() == QLatin1String("CRS")
		  || reader.name() == QLatin1String("SRS"))
			CRSs.append(reader.readElementText());
		else if (reader.name() == QLatin1String("Style"))
			styles.append(style(reader));
		else if (reader.name() == QLatin1String("ScaleHint")) {
			QXmlStreamAttributes attr = reader.attributes();
			double minHint = attr.value("min").toString().toDouble();
			double maxHint = attr.value("max").toString().toDouble();
			if (minHint > 0)
				scaleDenominator.setMin(hint2denominator(minHint));
			if (maxHint > 0)
				scaleDenominator.setMax(hint2denominator(maxHint));
			reader.skipCurrentElement();
		} else if (reader.name() == QLatin1String("MinScaleDenominator")) {
			double sd = reader.readElementText().toDouble();
			if (sd > 0)
				scaleDenominator.setMin(sd);
		} else if (reader.name() == QLatin1String("MaxScaleDenominator")) {
			double sd = reader.readElementText().toDouble();
			if (sd > 0)
				scaleDenominator.setMax(sd);
		} else if (reader.name() == QLatin1String("LatLonBoundingBox")) {
			QXmlStreamAttributes attr = reader.attributes();
			boundingBox = RectC(Coordinates(
			  attr.value("minx").toString().toDouble(),
			  attr.value("maxy").toString().toDouble()),
			  Coordinates(attr.value("maxx").toString().toDouble(),
			  attr.value("miny").toString().toDouble()));
			reader.skipCurrentElement();
		} else if (reader.name() == QLatin1String("EX_GeographicBoundingBox"))
			boundingBox = geographicBoundingBox(reader);
		else if (reader.name() == QLatin1String("Layer"))
			layer(reader, ctx, CRSs, styles, scaleDenominator, boundingBox);
		else
			reader.skipCurrentElement();
	}

	if ((index = ctx.layers.indexOf(name)) >= 0) {
		Layer &layer = ctx.layers[index];
		layer.scaleDenominator = scaleDenominator;
		layer.boundingBox = boundingBox;
		layer.isDefined = true;
		layer.hasStyle = styles.contains(layer.style) || layer.style.isEmpty();
		layer.hasCRS = CRSs.contains(ctx.setup.crs());
	}
}

void WMS::capability(QXmlStreamReader &reader, CTX &ctx)
{
	QList<QString> CRSs;
	QList<QString> styles;
	RangeF scaleDenominator(133.295598991, 559082264.0287178);
	RectC boundingBox;

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Layer"))
			layer(reader, ctx, CRSs, styles, scaleDenominator, boundingBox);
		else if (reader.name() == QLatin1String("Request"))
			request(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

void WMS::capabilities(QXmlStreamReader &reader, CTX &ctx)
{
	_version = reader.attributes().value("version").toString();

	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Capability"))
			capability(reader, ctx);
		else
			reader.skipCurrentElement();
	}
}

bool WMS::parseCapabilities()
{
	QFile file(_path);
	CTX ctx(_setup);
	QXmlStreamReader reader;


	if (ctx.layers.isEmpty()) {
		_errorString = "Invalid layers/styles list definition";
		return false;
	}

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_errorString = file.errorString();
		return false;
	}

	reader.setDevice(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("WMS_Capabilities")
		  || reader.name() == QLatin1String("WMT_MS_Capabilities"))
			capabilities(reader, ctx);
		else
			reader.raiseError("Not a WMS Capabilities XML file");
	}
	if (reader.error()) {
		_errorString = QString("%1:%2: %3").arg(_path).arg(reader.lineNumber())
		  .arg(reader.errorString());
		return false;
	}

	if (!ctx.formatSupported) {
		_errorString = ctx.setup.format() + ": format not provided";
		return false;
	}

	for (int i = 0; i < ctx.layers.size(); i++) {
		const Layer &layer = ctx.layers.at(i);

		if (!layer.isDefined) {
			_errorString = layer.name + ": layer not provided";
			return false;
		}
		if (!layer.hasStyle) {
			_errorString = layer.style + ": style not provided for layer "
			  + layer.name;
			return false;
		}
		if (!layer.hasCRS) {
			_errorString = ctx.setup.crs() + ": CRS not provided for layer "
			  + layer.name;
			return false;
		}
		if (!layer.scaleDenominator.isValid()
		  || layer.scaleDenominator.isNull()) {
			_errorString = "Invalid scale denominator range for layer "
			  + layer.name;
			return false;
		}
		if (!layer.boundingBox.isValid()) {
			_errorString = "Invalid/missing bounding box for layer "
			  + layer.name;
			return false;
		}
	}

	_projection = CRS::projection(ctx.setup.crs());
	if (!_projection.isValid()) {
		_errorString = ctx.setup.crs() + ": unknown CRS";
		return false;
	}

	_bbox = ctx.layers.first().boundingBox;
	for (int i = 1; i < ctx.layers.size(); i++)
		_bbox &= ctx.layers.at(i).boundingBox;
	if (_bbox.isNull()) {
		_errorString = "Empty layers bounding box join";
		return false;
	}

	_scaleDenominator = ctx.layers.first().scaleDenominator;
	for (int i = 1; i < ctx.layers.size(); i++)
		_scaleDenominator &= ctx.layers.at(i).scaleDenominator;
	if (_scaleDenominator.isNull()) {
		_errorString = "Empty layers scale denominator range join";
		return false;
	}

	if (_version >= "1.3.0") {
		if (_setup.coordinateSystem().axisOrder() == CoordinateSystem::Unknown)
			_cs = _projection.coordinateSystem();
		else
			_cs = _setup.coordinateSystem();
	} else
		_cs = CoordinateSystem::XY;

	_getMapUrl = ctx.url.isEmpty() ? _setup.url() : ctx.url;

	return true;
}

bool WMS::downloadCapabilities(const QString &url)
{
	if (!_downloader) {
		_downloader = new Downloader(this);
		connect(_downloader, &Downloader::finished, this,
		  &WMS::capabilitiesReady);
	}

	QList<Download> dl;
	dl.append(Download(url, _path));

	return _downloader->get(dl, _setup.authorization());
}

void WMS::capabilitiesReady()
{
	if (!QFileInfo(_path).exists()) {
		_errorString = "Error downloading capabilities XML file";
		_valid = false;
	} else {
		_ready = true;
		_valid = parseCapabilities();
	}

	emit downloadFinished();
}

WMS::WMS(const QString &file, const WMS::Setup &setup, QObject *parent)
  : QObject(parent), _setup(setup), _path(file), _downloader(0), _valid(false),
  _ready(false)
{
	QString url = QString("%1%2service=WMS&request=GetCapabilities")
	  .arg(setup.url(), setup.url().contains('?') ? "&" : "?");

	if (!QFileInfo(file).exists())
		_valid = downloadCapabilities(url);
	else {
		_ready = true;
		_valid = parseCapabilities();
	}
}
