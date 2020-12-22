#ifndef KMZMAP_H
#define KMZMAP_H

#include "projection.h"
#include "transform.h"
#include "rectd.h"
#include "map.h"

class QXmlStreamReader;
class QZipReader;
class Image;

class KMZMap : public Map
{
	Q_OBJECT

public:
	KMZMap(const QString &fileName, QObject *parent = 0);

	QString name() const;

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	struct Overlay {
		QString path;
		QSize size;
		RectC bbox;
		double rotation;
		Image *img;

		Transform transform;

		Overlay(const QString &path, const QSize &size, const RectC &bbox,
		  double rotation);
		bool operator==(const Overlay &other) const
		  {return path == other.path;}

		QPointF ll2xy(const Coordinates &c) const
		  {return QPointF(transform.proj2img(projection().ll2xy(c)));}
		Coordinates xy2ll(const QPointF &p) const
		  {return projection().xy2ll(transform.img2proj(p));}

		QRectF bounds() const {return QRectF(QPointF(0, 0), size);}
		qreal resolution(const QRectF &rect) const;

		void draw(QPainter *painter, const QRectF &rect, Flags flags);

		void load(QZipReader *zip);
		void unload();

		static const Projection &projection();
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

	void kml(QXmlStreamReader &reader, QZipReader &zip);
	void document(QXmlStreamReader &reader, QZipReader &zip);
	void folder(QXmlStreamReader &reader, QZipReader &zip);
	void groundOverlay(QXmlStreamReader &reader, QZipReader &zip);
	RectC latLonBox(QXmlStreamReader &reader, double *rotation);
	QString icon(QXmlStreamReader &reader);
	double number(QXmlStreamReader &reader);

	void draw(QPainter *painter, const QRectF &rect, int mapIndex, Flags flags);
	void computeZooms();
	void computeBounds();

	static bool resCmp(const Overlay &m1, const Overlay &m2);
	static bool xCmp(const Overlay &m1, const Overlay &m2);
	static bool yCmp(const Overlay &m1, const Overlay &m2);

	QList<Overlay> _maps;
	QVector<Zoom> _zooms;
	QVector<Bounds> _bounds;
	int _zoom;
	int _mapIndex;
	QZipReader *_zip;

	bool _valid;
	QString _errorString;
};

#endif // KMZMAP_H
