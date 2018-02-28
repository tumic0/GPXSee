#ifndef OFFLINEMAP_H
#define OFFLINEMAP_H

#include <QTransform>
#include "projection.h"
#include "transform.h"
#include "linearunits.h"
#include "map.h"
#include "tar.h"
#include "ozf.h"

class QIODevice;
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
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const OfflineMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const OfflineMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect);

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	QPointF ll2pp(const Coordinates &c) const
	  {return _projection.ll2xy(c);}
	QPointF xy2pp(const QPointF &p) const
	  {return _inverted.map(p);}
	QPointF pp2xy(const QPointF &p) const
	  {return _transform.map(p);}

private:
	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	bool getTileInfo(const QStringList &tiles, const QString &path = QString());
	bool getImageInfo(const QString &path);
	bool totalSizeSet();

	void drawTiled(QPainter *painter, const QRectF &rect);
	void drawOZF(QPainter *painter, const QRectF &rect);
	void drawImage(QPainter *painter, const QRectF &rect);

	void rescale(int zoom);

	QString _name;

	Projection _projection;
	QTransform _transform, _inverted;

	OZF _ozf;
	Tar _tar;
	QString _tarPath;
	QImage *_img;
	QString _imgPath;
	QSize _tileSize;
	QString _tileName;
	QSize _size;

	int _zoom;
	QPointF _scale;

	bool _valid;
	QString _errorString;
};

#endif // OFFLINEMAP_H
