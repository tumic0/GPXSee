#ifndef WMTS_H
#define WMTS_H

#include <QSize>
#include <QRect>
#include <QSet>
#include <QList>
#include <QHash>
#include "common/hash.h"
#include "common/rectc.h"
#include "common/kv.h"
#include "downloader.h"
#include "projection.h"
#include "coordinatesystem.h"

class QXmlStreamReader;

class WMTS : public QObject
{
	Q_OBJECT

public:
	class Setup
	{
	public:
		Setup(const QString &url, const QString &layer, const QString &set,
		  const QString &style, const QString &format, bool rest,
		  const CoordinateSystem &cs,
		  const QList<KV<QString, QString> > &dimensions,
		  const QList<HTTPHeader> &headers)
			: _url(url), _layer(layer), _set(set), _style(style),
			  _format(format), _rest(rest), _cs(cs), _dimensions(dimensions),
			  _headers(headers) {}

		const QString &url() const {return _url;}
		const QList<HTTPHeader> &headers() const {return _headers;}
		const QString &layer() const {return _layer;}
		const QString &set() const {return _set;}
		const QString &style() const {return _style;}
		const QString &format() const {return _format;}
		bool rest() const {return _rest;}
		const CoordinateSystem &coordinateSystem() const {return _cs;}
		const QList<KV<QString, QString> > &dimensions() const
		  {return _dimensions;}

	private:
		QString _url;
		QString _layer;
		QString _set;
		QString _style;
		QString _format;
		bool _rest;
		CoordinateSystem _cs;
		QList<KV<QString, QString> > _dimensions;
		QList<HTTPHeader> _headers;
	};

	class Zoom
	{
	public:
		Zoom(const QString &id, double scaleDenominator, const PointD &topLeft,
		  const QSize &tile, const QSize &matrix, const QRect &limits) :
		  _id(id), _scaleDenominator(scaleDenominator), _topLeft(topLeft),
		  _tile(tile), _matrix(matrix), _limits(limits) {}
		bool operator<(const Zoom &other) const
		  {return _scaleDenominator > other._scaleDenominator;}

		const QString &id() const {return _id;}
		double scaleDenominator() const {return _scaleDenominator;}
		const PointD &topLeft() const {return _topLeft;}
		const QSize &tile() const {return _tile;}
		const QSize &matrix() const {return _matrix;}
		const QRect &limits() const {return _limits;}

	private:
		QString _id;
		double _scaleDenominator;
		PointD _topLeft;
		QSize _tile;
		QSize _matrix;
		QRect _limits;
	};


	WMTS(const QString &path, const Setup &setup, QObject *parent = 0);

	const RectC &bbox() const {return _bbox;}
	const QList<Zoom> &zooms() const {return _zooms;}
	const Projection &projection() const {return _projection;}
	const QString &tileUrl() const {return _tileUrl;}
	CoordinateSystem cs() const {return _cs;}

	bool isReady() const {return _valid && _ready;}
	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

signals:
	void downloadFinished();

private slots:
	void capabilitiesReady();

private:
	struct TileMatrix {
		QString id;
		double scaleDenominator;
		PointD topLeft;
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

	struct Layer {
		Layer()
		  : hasLayer(false), hasStyle(false), hasFormat(false), hasSet(false) {}

		QSet<MatrixLimits> limits;
		QString defaultStyle;
		RectC bbox;
		QString tileUrl;
		bool hasLayer;
		bool hasStyle;
		bool hasFormat;
		bool hasSet;
	};

	struct CTX {
		QSet<TileMatrix> matrixes;
		QString crs;
		Layer layer;
	};

	RectC wgs84BoundingBox(QXmlStreamReader &reader);
	MatrixLimits tileMatrixLimits(QXmlStreamReader &reader);
	TileMatrix tileMatrix(QXmlStreamReader &reader);
	QSet<MatrixLimits> tileMatrixSetLimits(QXmlStreamReader &reader);
	QString style(QXmlStreamReader &reader);
	void tileMatrixSet(QXmlStreamReader &reader, CTX &ctx);
	void tileMatrixSetLink(QXmlStreamReader &reader, Layer &layer);
	void layer(QXmlStreamReader &reader, CTX &ctx);
	void contents(QXmlStreamReader &reader, CTX &ctx);
	void capabilities(QXmlStreamReader &reader, CTX &ctx);
	bool parseCapabilities(CTX &ctx);
	void createZooms(const CTX &ctx);
	bool init();

	WMTS::Setup _setup;
	QString _path;
	RectC _bbox;
	QList<Zoom> _zooms;
	Projection _projection;
	QString _tileUrl;
	CoordinateSystem _cs;

	bool _valid, _ready;
	QString _errorString;

	friend HASH_T qHash(const WMTS::TileMatrix &key);
	friend HASH_T qHash(const WMTS::MatrixLimits &key);
};

inline HASH_T qHash(const WMTS::TileMatrix &key)
{
	return ::qHash(key.id);
}

inline HASH_T qHash(const WMTS::MatrixLimits &key)
{
	return ::qHash(key.id);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const WMTS::Setup &setup);
QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom);
#endif // QT_NO_DEBUG

#endif // WMTS_H
