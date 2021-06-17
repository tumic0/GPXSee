#ifndef WORLDFILEMAP_H
#define WORLDFILEMAP_H

#include "transform.h"
#include "projection.h"
#include "map.h"

class Image;

class WorldFileMap : public Map
{
	Q_OBJECT

public:
	WorldFileMap(const QString &fileName, const Projection &proj, QObject *parent = 0);
	~WorldFileMap();

	QRectF bounds();
	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load();
	void unload();
	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);
	void setInputProjection(const Projection &projection);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	Projection _projection;
	Transform _transform;
	Image *_img;
	QSize _size;
	qreal _ratio;
	QString _imgFile;
	bool _hasPRJ;

	bool _valid;
	QString _errorString;
};

#endif // WORLDFILEMAP_H
