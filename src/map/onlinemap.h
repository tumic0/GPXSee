#ifndef ONLINEMAP_H
#define ONLINEMAP_H

#include "common/coordinates.h"
#include "common/range.h"
#include "map.h"
#include "tile.h"

class Downloader;

class OnlineMap : public Map
{
	Q_OBJECT

public:
	OnlineMap(const QString &name, const QString &url, const Range &zooms,
	  QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return _zoom;}
	qreal zoomFit(const QSize &size, const RectC &br);
	qreal zoomFit(qreal resolution, const Coordinates &c);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const OnlineMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const OnlineMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect);

	void setBlockingMode(bool block) {_block = block;}
	void clearCache();

	static void setDownloader(Downloader *downloader)
	  {OnlineMap::downloader = downloader;}

	void load();
	void unload();

private slots:
	void emitLoaded();

private:
	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	void fillTile(Tile &tile);
	QString tileUrl(const Tile &tile) const;
	QString tileFile(const Tile &tile) const;
	void loadTilesAsync(QList<Tile> &list);
	void loadTilesSync(QList<Tile> &list);
	int limitZoom(int zoom) const;

	Range _zooms;
	int _zoom;
	QString _name;
	QString _url;
	bool _block;

	static Downloader *downloader;
};

#endif // ONLINEMAP_H
