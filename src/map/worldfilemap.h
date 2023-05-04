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
	WorldFileMap(const QString &fileName, QObject *parent = 0);
	~WorldFileMap();

	RectC llBounds(const Projection &proj);
	QRectF bounds();
	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, bool *isDir);

private:
	Projection _projection;
	Transform _transform;
	Image *_img;
	QSize _size;
	qreal _mapRatio;
	QString _imgFile;
	bool _hasPRJ;

	bool _valid;
	QString _errorString;
};

#endif // WORLDFILEMAP_H
