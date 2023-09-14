#ifndef ATLAS_H
#define ATLAS_H

#include "map.h"
#include "rectd.h"
#include "projection.h"

class OziMap;

class Atlas : public Map
{
	Q_OBJECT

public:
	Atlas(const QString &fileName, bool TAR, const Projection &proj,
	  QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds();

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

	static Map *createTAR(const QString &path, const Projection &proj,
	  bool *isDir);
	static Map *createTBA(const QString &path, const Projection &proj,
	  bool *isDir);

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

	friend QDebug operator<<(QDebug dbg, const Bounds &bounds);
	friend QDebug operator<<(QDebug dbg, const Zoom &zoom);

	QString _name;

	QList<OziMap*> _maps;
	QVector<Zoom> _zooms;
	QVector<Bounds> _bounds;
	int _zoom;
	int _mapIndex;

	bool _valid;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const Atlas::Zoom &zoom);
QDebug operator<<(QDebug dbg, const Atlas::Bounds &bounds);
#endif // QT_NO_DEBUG

#endif // ATLAS_H
