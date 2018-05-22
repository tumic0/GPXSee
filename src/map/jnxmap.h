#ifndef JNXMAP_H
#define JNXMAP_H

#include <QFile>
#include <QVector>
#include "common/rtree.h"
#include "common/rectc.h"
#include "transform.h"
#include "map.h"

class JNXMap : public Map
{
public:
	Q_OBJECT

public:
	JNXMap(const QString &fileName, QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QRectF &rect) const;

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
		{return static_cast<const JNXMap &>(*this).ll2xy(c);}
	Coordinates xy2ll(const QPointF &p)
		{return static_cast<const JNXMap &>(*this).xy2ll(p);}

	void draw(QPainter *painter, const QRectF &rect, bool block);

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

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

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

	bool _valid;
	QString _errorString;
};

#endif // JNXMAP_H
