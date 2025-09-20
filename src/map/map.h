#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QString>
#include <QRectF>
#include <QFlags>
#include "common/rectc.h"
#include "common/util.h"
#include "dem.h"


class QPainter;
class Projection;

class Map : public QObject
{
	Q_OBJECT

public:
	enum Flag {
		NoFlags = 0,
		Block = 1,
		OpenGL = 2,
		HillShading = 4,
	};
	Q_DECLARE_FLAGS(Flags, Flag)

	Map(const QString &path, QObject *parent = 0)
	  : QObject(parent), _path(path) {}
	virtual ~Map() {}

	const QString &path() const {return _path;}
	virtual QString name() const {return Util::file2name(path());}

	virtual bool isValid() const {return true;}
	virtual bool isReady() const {return true;}
	virtual QString errorString() const {return QString();}

	virtual void load(const Projection &, const Projection &, qreal, bool, int) {}
	virtual void unload() {}

	/* llBounds() is mandatory for maps that do not provide bounds() until
	   load() is called! */
	virtual RectC llBounds();
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

	virtual double elevation(const Coordinates &c) {return DEM::elevation(c);}

	virtual QStringList layers(const QString &, int &) const
	  {return QStringList();}
	virtual bool hillShading() const {return false;}

	virtual void clearCache() {}

signals:
	void tilesLoaded();
	void mapLoaded();

private:
	QString _path;
};

Q_DECLARE_METATYPE(Map*)
Q_DECLARE_OPERATORS_FOR_FLAGS(Map::Flags)

#endif // MAP_H
