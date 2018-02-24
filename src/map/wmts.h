#ifndef WMTS_H
#define WMTS_H

#include <QSize>
#include <QRect>
#include <QSet>
#include <QList>
#include <QHash>
#include "common/rectc.h"
#include "projection.h"

class QXmlStreamReader;
class Downloader;

class WMTS
{
public:
	struct Zoom {
		QString id;
		qreal scaleDenominator;
		QPointF topLeft;
		QSize tile;
		QSize matrix;
		QRect limits;

		Zoom() {}
		Zoom(const QString &id, qreal scaleDenominator, const QPointF &topLeft,
		  const QSize &tile, const QSize &matrix, const QRect &limits) :
		  id(id), scaleDenominator(scaleDenominator), topLeft(topLeft),
		  tile(tile), matrix(matrix), limits(limits) {}
		bool operator<(const Zoom &other) const
		  {return scaleDenominator > other.scaleDenominator;}
	};

	bool load(const QString &path, const QString &url, const QString &layer,
	  const QString &set);
	const QString &errorString() const {return _errorString;}

	const RectC &bounds() const {return _bounds;}
	QList<Zoom> zooms() const;
	const Projection &projection() const {return _projection;}

	static Downloader *downloader() {return _downloader;}
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

	bool createProjection(const QString &crs);

	RectC wgs84BoundingBox(QXmlStreamReader &reader);
	MatrixLimits tileMatrixLimits(QXmlStreamReader &reader);
	TileMatrix tileMatrix(QXmlStreamReader &reader);
	QSet<MatrixLimits> tileMatrixSetLimits(QXmlStreamReader &reader);
	void tileMatrixSet(QXmlStreamReader &reader, const QString &set);
	void tileMatrixSetLink(QXmlStreamReader &reader, const QString &set);
	void layer(QXmlStreamReader &reader, const QString &layer,
	  const QString &set);
	void contents(QXmlStreamReader &reader, const QString &layer,
	  const QString &set);
	void capabilities(QXmlStreamReader &reader, const QString &layer,
	  const QString &set);
	bool parseCapabilities(const QString &path, const QString &layer,
	  const QString &set);
	bool getCapabilities(const QString &url, const QString &file);

	QSet<TileMatrix> _matrixes;
	QSet<MatrixLimits> _limits;
	RectC _bounds;
	Projection _projection;

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
QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom);
#endif // QT_NO_DEBUG

#endif // WMTS_H
