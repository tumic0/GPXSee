#ifndef GEMFMAP_H
#define GEMFMAP_H

#include <QFile>
#include <QDebug>
#include "map.h"

class GEMFMap : public Map
{
public:
	Q_OBJECT

public:
	GEMFMap(const QString &fileName, QObject *parent = 0);

	QRectF bounds();
	RectC llBounds() {return _bounds;}

	int zoom() const {return _zi;}
	void setZoom(int zoom) {_zi = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();
	qreal resolution(const QRectF &rect);

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private:
	struct Region {
		quint32 minX;
		quint32 maxX;
		quint32 minY;
		quint32 maxY;
		quint64 offset;
	};

	struct Zoom {
		int level;
		QList<Region> ranges;

		Zoom(int level) : level(level) {}

		bool operator==(const Zoom &other) const
		  {return level == other.level;}
		bool operator<(const Zoom &other) const
		  {return level < other.level;}
	};

	QRect rect(const Zoom &zoom) const;
	bool readHeader(QDataStream &stream);
	bool readRegions(QDataStream &stream);
	bool computeBounds();
	qreal tileSize() const;
	QByteArray tileData(const QPoint &tile);
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	friend QDebug operator<<(QDebug dbg, const Region &region);
	friend QDebug operator<<(QDebug dbg, const Zoom &zoom);

	QFile _file;
	int _zi;
	RectC _bounds;
	qreal _mapRatio;
	int _tileSize;
	QList<Zoom> _zooms;

	bool _valid;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const GEMFMap::Region &region);
QDebug operator<<(QDebug dbg, const GEMFMap::Zoom &zoom);
#endif // QT_NO_DEBUG

#endif // GEMFMAP_H
