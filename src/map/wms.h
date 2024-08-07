#ifndef WMS_H
#define WMS_H

#include <QString>
#include <QRectF>
#include "common/range.h"
#include "common/rectc.h"
#include "common/kv.h"
#include "downloader.h"
#include "projection.h"
#include "coordinatesystem.h"

class QXmlStreamReader;

class WMS : public QObject
{
	Q_OBJECT

public:
	class Setup
	{
	public:
		Setup(const QString &url, const QString &layer, const QString &style,
		  const QString &format, const QString &crs, const CoordinateSystem &cs,
		  const QList<KV<QString, QString> > &dimensions,
		  const QList<HTTPHeader> &headers)
			: _url(url), _layer(layer), _style(style), _format(format),
			  _crs(crs), _cs(cs), _dimensions(dimensions),
			  _headers(headers) {}

		const QString &url() const {return _url;}
		const QList<HTTPHeader> &headers() const {return _headers;}
		const QString &layer() const {return _layer;}
		const QString &style() const {return _style;}
		const QString &format() const {return _format;}
		const QString &crs() const {return _crs;}
		const CoordinateSystem &coordinateSystem() const {return _cs;}
		const QList<KV<QString, QString> > &dimensions() const
		  {return _dimensions;}

	private:
		QString _url;
		QString _layer;
		QString _style;
		QString _format;
		QString _crs;
		CoordinateSystem _cs;
		QList<KV<QString, QString> > _dimensions;
		QList<HTTPHeader> _headers;
	};


	WMS(const QString &path, const Setup &setup, QObject *parent = 0);

	const RectC &bbox() const {return _bbox;}
	const Projection &projection() const {return _projection;}
	CoordinateSystem cs() const {return _cs;}
	const RangeF &scaleDenominator() const {return _scaleDenominator;}
	const QString &version() const {return _version;}
	const QString &getMapUrl() const {return _getMapUrl;}
	const WMS::Setup &setup() const {return _setup;}

	bool isReady() const {return _valid && _ready;}
	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

signals:
	void downloadFinished();

private slots:
	void capabilitiesReady();

private:
	struct Layer {
		QString name;
		QString style;
		RangeF scaleDenominator;
		RectC boundingBox;
		bool isDefined;
		bool hasStyle;
		bool hasCRS;

		Layer(const QString &name, const QString &style = QString())
		  : name(name), style(style), isDefined(false), hasStyle(false),
		  hasCRS(false) {}
		bool operator==(const Layer &other) const
		  {return this->name == other.name;}
	};

	struct CTX {
		const Setup &setup;
		QList<Layer> layers;
		bool formatSupported;
		QString url;

		CTX(const Setup &setup);
	};

	RectC geographicBoundingBox(QXmlStreamReader &reader);
	QString style(QXmlStreamReader &reader);
	void get(QXmlStreamReader &reader, CTX &ctx);
	void http(QXmlStreamReader &reader, CTX &ctx);
	void dcpType(QXmlStreamReader &reader, CTX &ctx);
	void getMap(QXmlStreamReader &reader, CTX &ctx);
	void request(QXmlStreamReader &reader, CTX &ctx);
	void layer(QXmlStreamReader &reader, CTX &ctx, const QList<QString> &pCRSs,
	  const QList<QString> &pStyles, RangeF &pScaleDenominator,
	  RectC &pBoundingBox);
	void capability(QXmlStreamReader &reader, CTX &ctx);
	void capabilities(QXmlStreamReader &reader, CTX &ctx);
	bool parseCapabilities();

	WMS::Setup _setup;
	QString _path;
	Projection _projection;
	RangeF _scaleDenominator;
	RectC _bbox;
	QString _version;
	QString _getMapUrl;
	CoordinateSystem _cs;

	bool _valid, _ready;
	QString _errorString;
};

#endif // WMS_H
