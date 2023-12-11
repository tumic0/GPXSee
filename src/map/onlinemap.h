#ifndef ONLINEMAP_H
#define ONLINEMAP_H

#include <QImageReader>
#include <QPixmap>
#include "common/range.h"
#include "common/rectc.h"
#include "map.h"
#include "tileloader.h"

class OnlineTile
{
public:
	OnlineTile(const QPoint &xy, const QString &file, int zoom, int overzoom,
	  int scaledSize) : _xy(xy), _file(file), _zoom(zoom), _overzoom(overzoom),
	  _scaledSize(scaledSize) {}

	void load()
	{
		QByteArray format(_overzoom
		  ? QByteArray::number(_zoom) + ';' + QByteArray::number(_overzoom)
		  : QByteArray::number(_zoom));
		QImageReader reader(_file, format);
		if (_scaledSize)
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
		_pixmap = QPixmap::fromImage(reader.read());
	}

	const QPoint &xy() const {return _xy;}
	const QString &file() const {return _file;}
	const QPixmap &pixmap() const {return _pixmap;}

private:
	QPoint _xy;
	QString _file;
	int _zoom;
	int _overzoom;
	int _scaledSize;
	QPixmap _pixmap;
};

class OnlineMap : public Map
{
	Q_OBJECT

public:
	OnlineMap(const QString &fileName, const QString &name, const QString &url,
	  const Range &zooms, const RectC &bounds, qreal tileRatio,
	  const QList<HTTPHeader> &headers, int tileSize, bool scalable,
	  bool invertY, bool quadTiles, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void clearCache();

private:
	int limitZoom(int zoom) const;
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QPoint tileCoordinates(int x, int y, int zoom);
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	TileLoader *_tileLoader;
	QString _name;
	Range _zooms;
	RectC _bounds;
	int _zoom;
	int _tileSize;
	int _base;
	qreal _mapRatio, _tileRatio;
	bool _scalable;
	int _scaledSize;
	bool _invertY;
};

#endif // ONLINEMAP_H
