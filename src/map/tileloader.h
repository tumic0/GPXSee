#ifndef TILELOADER_H
#define TILELOADER_H

#include <QObject>
#include <QString>
#include "common/downloader.h"
#include "rectd.h"

class TileLoader : public QObject
{
	Q_OBJECT

public:
	enum UrlType {
		XYZ,
		QuadTiles,
		BoundingBox
	};

	class Tile
	{
	public:
		Tile() {}
		Tile(const QPoint &xy, int zoom)
		  : _xy(xy), _zoom(zoom) {}
		Tile(const QPoint &xy, const QString &zoom)
		  : _xy(xy), _zoom(zoom) {}
		Tile(const QPoint &xy, int zoom, const RectD &bbox)
		  : _xy(xy), _zoom(zoom), _bbox(bbox) {}

		const QVariant &zoom() const {return _zoom;}
		const QPoint &xy() const {return _xy;}
		const RectD &bbox() const {return _bbox;}
		const QString &file() const {return _file;}

	private:
		friend class TileLoader;

		void setFile(const QString &file) {_file = file;}

		QPoint _xy;
		QVariant _zoom;
		RectD _bbox;
		QString _file;
	};


	TileLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url, UrlType type) {_url = url; _urlType = type;}
	void setHeaders(const QList<HTTPHeader> &headers) {_headers = headers;}

	void loadTilesAsync(QVector<Tile> &list);
	void loadTilesSync(QVector<Tile> &list);
	void clearCache();

signals:
	void finished();

private:
	QUrl tileUrl(const Tile &tile) const;
	QString tileFile(const Tile &tile) const;

	Downloader *_downloader;
	QString _url;
	UrlType _urlType;
	QString _dir;
	QList<HTTPHeader> _headers;
};

#endif // TILELOADER_H
