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

	QString name() const {return _name;}

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setDevicePixelRatio(qreal ratio) {_ratio = ratio;}

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	struct Tile {
		QPointF pos;
		quint32 size;
		quint32 offset;
	};

	struct Zoom {
		Transform transform;
		QVector<Tile> tiles;
		RTree<Tile*, qreal, 2> tree;
	};

	template<class T> bool readValue(T &val);
	bool readString(QByteArray &ba);
	bool readTiles();

	static bool cb(Tile *tile, void *context);
	static QPixmap pixmap(const Tile *tile, QFile *file);

	QString _name;
	QFile _file;
	QVector<Zoom> _zooms;
	int _zoom;
	RectC _bounds;
	Projection _projection;
	qreal _ratio;

	bool _valid;
	QString _errorString;
};

#endif // JNXMAP_H
