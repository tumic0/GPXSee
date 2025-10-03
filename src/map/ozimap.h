#ifndef OZIMAP_H
#define OZIMAP_H

#include "transform.h"
#include "projection.h"
#include "calibrationpoint.h"
#include "map.h"

class Tar;
class OZF;
class Image;

class OziMap : public Map
{
	Q_OBJECT

public:
	enum CalibrationType {
		Unknown, MAP, GMI
	};

	OziMap(const QString &fileName, CalibrationType type, const Projection &proj,
	  QObject *parent = 0);
	OziMap(const QString &dirName, Tar &tar, const Projection &proj,
	  QObject *parent = 0);
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

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi, int style, int layer);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	PointD ll2pp(const Coordinates &c) const
	  {return _projection.ll2xy(c);}
	PointD xy2pp(const QPointF &p) const
	  {return _transform.img2proj(p * _mapRatio);}
	QPointF pp2xy(const PointD &p) const
	  {return _transform.proj2img(p) / _mapRatio;}

	static Map *createTAR(const QString &path, const Projection &proj,
	  bool *isDir);
	static Map *createMAP(const QString &path, const Projection &proj,
	  bool *isDir);
	static Map *createGMI(const QString &path, const Projection &proj,
	  bool *isDir);

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
	void computeTransform();

	static QString calibrationFile(const QStringList &files, const QString path,
	  CalibrationType &type);

	QString _name;
	Projection _projection;
	Transform _transform;
	Image *_img;
	Tar *_tar;
	OZF *_ozf;
	ImageInfo _map, _tile;
	int _zoom;
	QPointF _scale;
	qreal _mapRatio;
	QList<CalibrationPoint> _calibrationPoints;

	bool _valid;
	QString _errorString;
};

#endif // OZIMAP_H
