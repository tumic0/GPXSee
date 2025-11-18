#ifndef TILELOADER_H
#define TILELOADER_H

#include <QObject>
#include <QString>
#include <QDir>
#include "downloader.h"
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
		const QStringList &files() const {return _files;}

		bool isComplete() const
		{
			for (int i = 0; i < _files.size(); i++)
				if (_files.at(i).isNull())
					return false;
			return true;
		}

	private:
		friend class TileLoader;

		void addFile(const QString &file) {_files.append(file);}
		void setFile(int layer, const QString &file) {_files[layer] = file;}

		QPoint _xy;
		QVariant _zoom;
		RectD _bbox;
		QStringList _files;
	};


	TileLoader(const QString &dir, QObject *parent = 0);

	void setUrl(const QString &url, UrlType type)
	  {_url.append(url); _urlType = type;}
	void setUrl(const QStringList &url, UrlType type)
	  {_url = url; _urlType = type;}
	void setHeaders(const QList<HTTPHeader> &headers)
	  {_headers = headers;}

	void loadTilesAsync(QVector<Tile> &list);
	void loadTilesSync(QVector<Tile> &list);
	void clearCache();

signals:
	void finished();

private:
	QUrl tileUrl(const Tile &tile, int layer) const;
	QString tileFile(const Tile &tile, int layer) const;

	Downloader *_downloader;
	QStringList _url;
	UrlType _urlType;
	QDir _dir;
	QList<HTTPHeader> _headers;
};

#endif // TILELOADER_H
