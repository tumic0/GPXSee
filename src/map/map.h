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

	virtual int zoom() const {return 0;}
	virtual void setZoom(int) {}
	virtual int zoomFit(const QSize &, const RectC &) {return 0;}
	virtual int zoomIn() {return 0;}
	virtual int zoomOut() {return 0;}

	virtual QPointF ll2xy(const Coordinates &c) = 0;
	virtual Coordinates xy2ll(const QPointF &p) = 0;

	virtual void draw(QPainter *painter, const QRectF &rect, Flags flags) = 0;

	virtual void clearCache() {}
	virtual void load() {}
	virtual void unload() {}
	virtual void setDevicePixelRatio(qreal) {}

	virtual bool isValid() const {return true;}
	virtual QString errorString() const {return QString();}

signals:
	void loaded();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Map::Flags)

#endif // MAP_H
