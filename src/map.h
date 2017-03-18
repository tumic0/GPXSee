#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QString>
#include <QRectF>

class QPainter;
class Coordinates;

class Map : public QObject
{
	Q_OBJECT

public:
	Map(QObject *parent = 0) : QObject(parent) {}

	virtual const QString &name() const = 0;

	virtual QRectF bounds() const = 0;
	virtual qreal resolution(const QPointF &p) const = 0;

	virtual qreal zoom() const = 0;
	virtual qreal zoomFit(const QSize &size, const QRectF &br) = 0;
	virtual qreal zoomIn() = 0;
	virtual qreal zoomOut() = 0;

	virtual QPointF ll2xy(const Coordinates &c) const = 0;
	virtual Coordinates xy2ll(const QPointF &p) const = 0;

	virtual void draw(QPainter *painter, const QRectF &rect) = 0;

	virtual void setBlockingMode(bool block) {Q_UNUSED(block);}
	virtual void clearCache() {}
	virtual void load() {}
	virtual void unload() {}

signals:
	void loaded();
};

#endif // MAP_H
