#ifndef WMTS_H
#define WMTS_H

#include <QSize>
#include <QMap>
#include <QList>
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

		Zoom() : scaleDenominator(0) {}
		bool operator<(const Zoom &other) const
			{return this->scaleDenominator > other.scaleDenominator;}
		bool isValid() const
			{return scaleDenominator > 0 && !id.isEmpty() && tile.isValid()
			  && matrix.isValid();}
	};

	bool load(const QString &path, const QString &url, const QString &layer,
	  const QString &set);
	const QString &errorString() const {return _errorString;}

	QList<Zoom> zooms() const;
	const Projection &projection() const {return _projection;}

	static Downloader *downloader() {return _downloader;}
	static void setDownloader(Downloader *downloader)
	  {_downloader = downloader;}

private:
	bool createProjection(const QString &crs);

	void tileMatrixLimits(QXmlStreamReader &reader);
	void tileMatrix(QXmlStreamReader &reader);
	void tileMatrixSetLimits(QXmlStreamReader &reader);
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

	QMap<QString, Zoom> _zooms;
	Projection _projection;

	QString _errorString;

	static Downloader *_downloader;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const WMTS::Zoom &zoom);
#endif // QT_NO_DEBUG

#endif // WMTS_H
