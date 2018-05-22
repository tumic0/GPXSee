#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QString>
#include <QRectF>
#include <QColor>
#include "common/coordinates.h"

class QPainter;
class RectC;

class Map : public QObject
{
	Q_OBJECT

public:
	Map(QObject *parent = 0) : QObject(parent) {}
	virtual ~Map() {}

	virtual const QString &name() const = 0;

	virtual QRectF bounds() const = 0;
	virtual qreal resolution(const QRectF &rect) const = 0;

	virtual int zoom() const = 0;
	virtual void setZoom(int zoom) = 0;
	virtual int zoomFit(const QSize &size, const RectC &rect) = 0;
	virtual int zoomIn() = 0;
	virtual int zoomOut() = 0;

	virtual QPointF ll2xy(const Coordinates &c) = 0;
	virtual Coordinates xy2ll(const QPointF &p) = 0;

	virtual void draw(QPainter *painter, const QRectF &rect, bool block) = 0;

	virtual void clearCache() {}
	virtual void load() {}
	virtual void unload() {}

	virtual bool isValid() const {return true;}
	virtual QString errorString() const {return QString();}

signals:
	void loaded();
};

#endif // MAP_H
