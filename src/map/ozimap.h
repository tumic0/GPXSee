#ifndef OZIMAP_H
#define OZIMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"

class Tar;
class OZF;
class Image;

class OziMap : public Map
{
	Q_OBJECT

public:
	OziMap(const QString &fileName, QObject *parent = 0);
	OziMap(const QString &fileName, Tar &tar, QObject *parent = 0);
	~OziMap();

	QString name() const {return _name;}

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setDevicePixelRatio(qreal ratio);
	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	PointD ll2pp(const Coordinates &c) const
	  {return _projection.ll2xy(c);}
	PointD xy2pp(const QPointF &p) const
	  {return _transform.img2proj(p * _ratio);}
	QPointF pp2xy(const PointD &p) const
	  {return _transform.proj2img(p) / _ratio;}

private:
	struct ImageInfo {
		QSize size;
		QString path;

		bool isValid() const {return size.isValid() && !path.isEmpty();}
	};

	bool setTileInfo(const QStringList &tiles, const QString &path = QString());
	bool setImageInfo(const QString &path);

	void drawTiled(QPainter *painter, const QRectF &rect) const;
	void drawOZF(QPainter *painter, const QRectF &rect) const;
	void drawImage(QPainter *painter, const QRectF &rect, Flags flags) const;

	void rescale(int zoom);

	QString _name;
	Projection _projection;
	Transform _transform;
	Image *_img;
	Tar *_tar;
	OZF *_ozf;
	ImageInfo _map, _tile;
	int _zoom;
	QPointF _scale;
	qreal _ratio;

	bool _valid;
	QString _errorString;
};

#endif // OZIMAP_H
