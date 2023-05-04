#ifndef JNXMAP_H
#define JNXMAP_H

#include <QFile>
#include <QVector>
#include "common/rtree.h"
#include "common/rectc.h"
#include "transform.h"
#include "projection.h"
#include "map.h"

class JNXMap : public Map
{
public:
	Q_OBJECT

public:
	JNXMap(const QString &fileName, QObject *parent = 0);
	~JNXMap();

	QRectF bounds();
	RectC llBounds(const Projection &) {return _bounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, bool *isDir);

private:
	struct Tile {
		qint32 top, right, bottom, left;
		quint16 width, height;
		quint32 size;
		quint32 offset;

		QPointF pos;
	};

	struct Level {
		quint32 count;
		quint32 offset;
		quint32 scale;
	};

	struct Zoom {
		Zoom() {}
		Zoom(const Level &level) : level(level) {}

		Level level;
		Transform transform;
		QVector<Tile> tiles;
		RTree<Tile*, qreal, 2> tree;
	};

	struct Ctx {
		QPainter *painter;
		QFile *file;
		qreal ratio;

		Ctx(QPainter *painter, QFile *file, qreal ratio)
		  : painter(painter), file(file), ratio(ratio) {}
	};


	template<class T> bool readValue(T &val);
	bool readString(QByteArray &ba);
	bool readHeader();
	bool readTiles();
	void clearTiles();

	static bool cb(Tile *tile, void *context);
	static QPixmap pixmap(const Tile *tile, QFile *file);

	QFile _file;
	QList<Zoom*> _zooms;
	int _zoom;
	RectC _bounds;
	Projection _projection;
	qreal _mapRatio;

	bool _valid;
	QString _errorString;
};

#endif // JNXMAP_H
