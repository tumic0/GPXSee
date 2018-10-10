#ifndef ATLAS_H
#define ATLAS_H

#include "map.h"
#include "rectd.h"

class OziMap;

class Atlas : public Map
{
	Q_OBJECT

public:
	Atlas(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setDevicePixelRatio(qreal ratio);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static bool isAtlas(const QString &path);

private:
	struct Zoom {
		int first;
		int last;

		Zoom() : first(-1), last(-1) {}
		Zoom(int first, int last) : first(first), last(last) {}
	};

	struct Bounds {
		RectD pp;
		QRectF xy;

		Bounds() {}
		Bounds(const RectD &pp, const QRectF &xy) : pp(pp), xy(xy) {}
	};

	void draw(QPainter *painter, const QRectF &rect, int mapIndex, Flags flags);
	void computeZooms();
	void computeBounds();

	QString _name;

	QList<OziMap*> _maps;
	QVector<Zoom> _zooms;
	QVector<Bounds> _bounds;
	int _zoom;
	int _mapIndex;

	bool _valid;
	QString _errorString;
};

#endif // ATLAS_H
