#ifndef ONLINEMAP_H
#define ONLINEMAP_H

#include "map.h"
#include "tile.h"

class Downloader;

class OnlineMap : public Map
{
	Q_OBJECT

public:
	OnlineMap(const QString &name, const QString &url, QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return _scale;}
	qreal zoomFit(const QSize &size, const QRectF &br);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	void draw(QPainter *painter, const QRectF &rect);

	void setBlockingMode(bool block) {_block = block;}
	void clearCache();

	static void setDownloader(Downloader *downloader)
	  {OnlineMap::downloader = downloader;}

private slots:
	void emitLoaded();

private:
	QString tileUrl(const Tile &tile);
	QString tileFile(const Tile &tile);
	bool loadTileFile(Tile &tile, const QString &file);
	void fillTile(Tile &tile);
	void loadTilesAsync(QList<Tile> &list);
	void loadTilesSync(QList<Tile> &list);

	qreal _scale;
	QString _name;
	QString _url;
	bool _block;

	static Downloader *downloader;
};

#endif // ONLINEMAP_H
