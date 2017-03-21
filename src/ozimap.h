#ifndef OZIMAP_H
#define OZIMAP_H

#include <QImage>
#include <QTransform>
#include "map.h"
#include "tar.h"
#include "coordinates.h"

class QIODevice;
class QDir;

class OziMap : public Map
{
	Q_OBJECT

public:
	OziMap(const QString &path, QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return 1.0;}
	qreal zoomFit(const QSize &size, const QRectF &br);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	void draw(QPainter *painter, const QRectF &rect);

	void load();
	void unload();

	bool isValid() {return _valid;}

private:
	typedef QPair<QPoint, Coordinates> ReferencePoint;

	int parseMapFile(QIODevice &device, QList<ReferencePoint> &points);
	bool computeTransformation(const QList<ReferencePoint> &points);
	bool computeResolution(QList<ReferencePoint> &points);
	bool getTileName(const QStringList &tiles);
	bool getTileSize();

	QString _name;
	QString _imgPath;
	QSize _size;
	QTransform _transform;
	qreal _resolution;

	Tar _tar;
	QImage *_img;
	QSize _tileSize;
	QString _tileName;

	bool _valid;
};

#endif // OZIMAP_H
