#ifndef WMTS_H
#define WMTS_H

#include <QSize>
#include <QRect>
#include <QSet>
#include <QList>
#include <QHash>
#include "common/rectc.h"
#include "projection.h"
#include "downloader.h"
#include "coordinatesystem.h"

class QXmlStreamReader;

class WMTS
{
public:
	class Setup
	{
	public:
		Setup(const QString &url, const QString &layer, const QString &set,
		  const QString &style, const QString &format, bool rest,
		  const CoordinateSystem &cs,
		  const QList<QPair<QString, QString> > &dimensions,
		  const Authorization &authorization = Authorization())
			: _url(url), _layer(layer), _set(set), _style(style),
			  _format(format), _rest(rest), _cs(cs), _dimensions(dimensions),
			  _authorization(authorization) {}

		const QString &url() const {return _url;}
		const Authorization &authorization() const {return _authorization;}
		const QString &layer() const {return _layer;}
		const QString &set() const {return _set;}
		const QString &style() const {return _style;}
		const QString &format() const {return _format;}
		bool rest() const {return _rest;}
		const CoordinateSystem &coordinateSystem() const {return _cs;}
		const QList<QPair<QString, QString> > &dimensions() const
		  {return _dimensions;}

	private:
		QString _url;
		QString _layer;
		QString _set;
		QString _style;
		QString _format;
		bool _rest;
		CoordinateSystem _cs;
		QList<QPair<QString, QString> > _dimensions;
		Authorization _authorization;
	};

	class Zoom
	{
	public:
		Zoom(const QString &id, qreal scaleDenominator, const QPointF &topLeft,
		  const QSize &tile, const QSize &matrix, const QRect &limits) :
		  _id(id), _scaleDenominator(scaleDenominator), _topLeft(topLeft),
		  _tile(tile), _matrix(matrix), _limits(limits) {}
		bool operator<(const Zoom &other) const
		  {return _scaleDenominator > other._scaleDenominator;}

		const QString &id() const {return _id;}
		qreal scaleDenominator() const {return _scaleDenominator;}
		const QPointF &topLeft() const {return _topLeft;}
		const QSize &tile() const {return _tile;}
		const QSize &matrix() const {return _matrix;}
		const QRect &limits() const {return _limits;}

	private:
		QString _id;
		qreal _scaleDenominator;
		QPointF _topLeft;
		QSize _tile;
		QSize _matrix;
		QRect _limits;
	};


	WMTS(const QString &path, const Setup &setup);

	const RectC &bounds() const {return _bounds;}
	QList<Zoom> zooms() const;
	const Projection &projection() const {return _projection;}
	const QString &tileUrl() const {return _tileUrl;}

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

	static void setDownloader(Downloader *downloader)
	  {_downloader = downloader;}

private:
	struct TileMatrix {
		QString id;
		qreal scaleDenominator;
		QPointF topLeft;
		QSize tile;
		QSize matrix;

		TileMatrix() : scaleDenominator(0) {}
		bool operator==(const TileMatrix &other) const
		  {return this->id == other.id;}
		bool isValid() const
		  {return !id.isEmpty() && scaleDenominator > 0 && tile.isValid()
		    && matrix.isValid();}
	};

	struct MatrixLimits {
		QString id;
		QRect rect;

		MatrixLimits() {}
		MatrixLimits(const QString &id) : id(id) {}
		bool operator==(const MatrixLimits &other) const
		  {return this->id == other.id;}
		bool isValid() const
		  {return !id.isEmpty() && rect.isValid();}
	};

	struct CTX {
		const Setup &setup;
		QString crs;
		bool hasLayer;
		bool hasStyle;
		bool hasFormat;
		bool hasSet;

		CTX(const Setup &setup) : setup(setup), hasLayer(false), hasStyle(false),
		  hasFormat(false), hasSet(false) {}
	};

	RectC wgs84BoundingBox(QXmlStreamReader &reader);
	MatrixLimits tileMatrixLimits(QXmlStreamReader &reader);
	TileMatrix tileMatrix(QXmlStreamReader &reader);
	QSet<MatrixLimits> tileMatrixSetLimits(QXmlStreamReader &reader);
	QString style(QXmlStreamReader &reader);
	void tileMatrixSet(QXmlStreamReader &reader, CTX &ctx);
	void tileMatrixSetLink(QXmlStreamReader &reader, CTX &ctx);
	void layer(QXmlStreamReader &reader, CTX &ctx);
	void contents(QXmlStreamReader &reader, CTX &ctx);
	void capabilities(QXmlStreamReader &reader, CTX &ctx);
	bool parseCapabilities(const QString &path, const Setup &setup);
	bool getCapabilities(const QString &url, const QString &file,
	  const Authorization &authorization);

	QSet<TileMatrix> _matrixes;
	QSet<MatrixLimits> _limits;
	RectC _bounds;
	Projection _projection;
	QString _tileUrl;

	bool _valid;
	QString _errorString;

	static Downloader *_downloader;

	friend uint qHash(const WMTS::TileMatrix &key);
	friend uint qHash(const WMTS::MatrixLimits &key);
};

inline uint qHash(const WMTS::TileMatrix &key)
{
	return ::qHash(key.id);
}

inline uint qHash(const WMTS::MatrixLimits &key)
{
	return ::qHash(key.id);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const WMTS::Setup &setup);
QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom);
#endif // QT_NO_DEBUG

#endif // WMTS_H
