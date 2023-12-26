#ifndef KMZMAP_H
#define KMZMAP_H

#include <QImage>
#include "projection.h"
#include "transform.h"
#include "map.h"

class QXmlStreamReader;
class QZipReader;

class KMZMap : public Map
{
	Q_OBJECT

public:
	KMZMap(const QString &fileName, QObject *parent = 0);
	~KMZMap();

	RectC llBounds() {return _llbounds;}
	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private:
	class Overlay {
	public:
		Overlay(const QString &path, const RectC &bbox, double rotation)
		  : _path(path), _bbox(bbox), _rotation(rotation) {}

		const QString &path() const {return _path;}
		const RectC &bbox() const {return _bbox;}
		qreal rotation() const {return _rotation;}

	private:
		QString _path;
		RectC _bbox;
		qreal _rotation;
	};

	class Tile {
	public:
		Tile(const Overlay &overlay, QZipReader &zip);

		bool operator==(const Tile &other) const
		  {return _overlay.path() == other._overlay.path();}

		bool isValid() const {return _size.isValid();}
		const QString &path() const {return _overlay.path();}
		qreal rotation() const {return _overlay.rotation();}
		const RectC &bbox() const {return _overlay.bbox();}
		const Transform &transform() const {return _transform;}
		QRectF bounds() const;
		qreal resolution() const;

		void configure(const Projection &proj);

	private:
		Overlay _overlay;
		QSize _size;
		Transform _transform;
	};

	struct Zoom {
		int first;
		int last;

		Zoom() : first(-1), last(-1) {}
		Zoom(int first, int last) : first(first), last(last) {}
	};

	struct Bounds {
		RectC ll;
		QRectF xy;

		Bounds() {}
		Bounds(const RectC &ll, const QRectF &xy) : ll(ll), xy(xy) {}
	};

	void kml(QXmlStreamReader &reader, QList<Overlay> &overlays);
	void document(QXmlStreamReader &reader, QList<Overlay> &overlays);
	void folder(QXmlStreamReader &reader, QList<Overlay> &overlays);
	void groundOverlay(QXmlStreamReader &reader, QList<Overlay> &overlays);
	RectC latLonBox(QXmlStreamReader &reader, double *rotation);
	QString icon(QXmlStreamReader &reader);
	double number(QXmlStreamReader &reader);

	void draw(QPainter *painter, const QRectF &rect, int mapIndex);

	bool createTiles(const QList<Overlay> &overlays, QZipReader &zip);
	void computeZooms();
	void computeBounds();
	void computeLLBounds();

	QPointF ll2xy(const Coordinates &c, const Transform &transform) const
	  {return QPointF(transform.proj2img(_projection.ll2xy(c))) / _mapRatio;}
	Coordinates xy2ll(const QPointF &p, const Transform &transform) const
	  {return _projection.xy2ll(transform.img2proj(p * _mapRatio));}

	static bool resCmp(const Tile &m1, const Tile &m2);
	static bool xCmp(const Tile &m1, const Tile &m2);
	static bool yCmp(const Tile &m1, const Tile &m2);

	RectC _llbounds;
	QList<Tile> _tiles;
	QVector<Zoom> _zooms;
	QVector<Bounds> _bounds;
	int _zoom;
	int _mapIndex;
	QZipReader *_zip;
	qreal _adjust;
	Projection _projection;
	qreal _mapRatio;

	bool _valid;
	QString _errorString;
};

#endif // KMZMAP_H
