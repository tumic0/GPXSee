#ifndef OFFLINEMAP_H
#define OFFLINEMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"

class Tar;
class OZF;
class QImage;

class OfflineMap : public Map
{
	Q_OBJECT

public:
	OfflineMap(const QString &fileName, QObject *parent = 0);
	OfflineMap(const QString &fileName, Tar &tar, QObject *parent = 0);
	~OfflineMap();

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QRectF &rect) const;

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const OfflineMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const OfflineMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect, bool block);

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	PointD ll2pp(const Coordinates &c) const
	  {return _projection.ll2xy(c);}
	PointD xy2pp(const QPointF &p) const
	  {return _transform.img2proj(p);}
	QPointF pp2xy(const PointD &p) const
	  {return _transform.proj2img(p);}

private:
	struct ImageInfo {
		QSize size;
		QString path;

		bool isValid() const {return size.isValid() && !path.isEmpty();}
	};

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	bool setTileInfo(const QStringList &tiles, const QString &path = QString());
	bool setImageInfo(const QString &path);

	void drawTiled(QPainter *painter, const QRectF &rect) const;
	void drawOZF(QPainter *painter, const QRectF &rect) const;
	void drawImage(QPainter *painter, const QRectF &rect) const;

	void rescale(int zoom);

	QString _name;
	Projection _projection;
	Transform _transform;
	QImage *_img;
	Tar *_tar;
	OZF *_ozf;
	ImageInfo _map, _tile;
	int _zoom;
	QPointF _scale;

	bool _valid;
	QString _errorString;
};

#endif // OFFLINEMAP_H
