#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QString>
#include <QRectF>
#include <QFlags>
#include "common/coordinates.h"

class QPainter;
class RectC;

class Map : public QObject
{
	Q_OBJECT

public:
	enum Flag {
		NoFlags = 0,
		Block = 1,
		OpenGL = 2
	};
	Q_DECLARE_FLAGS(Flags, Flag)

	Map(QObject *parent = 0) : QObject(parent) {}
	virtual ~Map() {}

	virtual QString name() const = 0;

	virtual QRectF bounds() = 0;
	virtual qreal resolution(const QRectF &rect);

	virtual int zoom() const = 0;
	virtual void setZoom(int zoom) = 0;
	virtual int zoomFit(const QSize &size, const RectC &rect) = 0;
	virtual int zoomIn() = 0;
	virtual int zoomOut() = 0;

	virtual QPointF ll2xy(const Coordinates &c) = 0;
	virtual Coordinates xy2ll(const QPointF &p) = 0;

	virtual void draw(QPainter *painter, const QRectF &rect, Flags flags) = 0;

	virtual void clearCache() {}
	virtual void load() {}
	virtual void unload() {}
	virtual void setDevicePixelRatio(qreal ratio) {Q_UNUSED(ratio);}

	virtual bool isValid() const {return true;}
	virtual QString errorString() const {return QString();}

signals:
	void loaded();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Map::Flags)

#endif // MAP_H
