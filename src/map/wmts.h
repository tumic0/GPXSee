#ifndef WMTS_H
#define WMTS_H

#include <QSize>
#include <QList>
#include "projection.h"

class QXmlStreamReader;
class Downloader;

class WMTS
{
public:
	struct Zoom {
		qreal scaleDenominator;
		QPointF topLeft;
		QSize tile;
		QSize matrix;
	};

	bool load(const QString &path, const QString &url,
	  const QString &tileMatrixSet);
	const QString &errorString() const {return _errorString;}

	const QList<Zoom> &zooms() {return _zooms;}
	const Projection &projection() {return _projection;}

	static Downloader *downloader() {return _downloader;}
	static void setDownloader(Downloader *downloader)
	  {_downloader = downloader;}

private:
	bool createProjection(const QString &crs);

	void tileMatrix(QXmlStreamReader &reader);
	void tileMatrixSet(QXmlStreamReader &reader, const QString &set);
	void contents(QXmlStreamReader &reader, const QString &set);
	void capabilities(QXmlStreamReader &reader, const QString &set);
	bool parseCapabilities(const QString &path, const QString &tileMatrixSet);
	bool getCapabilities(const QString &url, const QString &file);

	QList<Zoom> _zooms;
	Projection _projection;

	QString _errorString;

	static Downloader *_downloader;
};

#endif // WMTS_H
