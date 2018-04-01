#ifndef WMS_H
#define WMS_H

#include <QString>
#include <QRectF>
#include "common/range.h"
#include "common/rectc.h"
#include "projection.h"
#include "downloader.h"

class QXmlStreamReader;

class WMS
{
public:
	class Setup
	{
	public:
		Setup(const QString &url, const QString &layer, const QString &style,
		  const QString &format, const QString &crs, bool yx,
		  const Authorization &authorization = Authorization())
		  : _url(url), _layer(layer), _style(style), _format(format), _crs(crs),
		  _yx(yx), _authorization(authorization) {}

		const QString &url() const {return _url;}
		const Authorization &authorization() const {return _authorization;}
		const QString &layer() const {return _layer;}
		const QString &style() const {return _style;}
		const QString &format() const {return _format;}
		const QString &crs() const {return _crs;}
		bool yx() const {return _yx;}

	private:
		QString _url;
		QString _layer;
		QString _style;
		QString _format;
		QString _crs;
		bool _yx;
		Authorization _authorization;
	};


	WMS(const QString &path, const Setup &setup);

	const Projection &projection() const {return _projection;}
	const RangeF &scaleDenominator() const {return _scaleDenominator;}
	const RectC &boundingBox() const {return _boundingBox;}
	const QString &version() const {return _version;}

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

	static void setDownloader(Downloader *downloader)
	  {_downloader = downloader;}

private:
	struct CTX {
		const Setup &setup;
		RangeF scaleDenominator;
		RectC boundingBox;
		bool layer;
		bool style;
		bool format;
		bool crs;

		CTX(const Setup &setup) : setup(setup), layer(false), style(false),
		  format(false), crs(false) {}
	};

	RectC geographicBoundingBox(QXmlStreamReader &reader);
	QString style(QXmlStreamReader &reader);
	void getMap(QXmlStreamReader &reader, CTX &ctx);
	void request(QXmlStreamReader &reader, CTX &ctx);
	void layer(QXmlStreamReader &reader, CTX &ctx, const QList<QString> &pCRSs,
	  const QList<QString> &pStyles, RangeF &pScaleDenominator,
	  RectC &pBoundingBox);
	void capability(QXmlStreamReader &reader, CTX &ctx);
	void capabilities(QXmlStreamReader &reader, CTX &ctx);
	bool parseCapabilities(const QString &path, const Setup &setup);
	bool getCapabilities(const QString &url, const QString &file,
	  const Authorization &authorization);

	Projection _projection;
	RangeF _scaleDenominator;
	RectC _boundingBox;
	QString _version;

	bool _valid;
	QString _errorString;

	static Downloader *_downloader;
};

#endif // WMS_H
