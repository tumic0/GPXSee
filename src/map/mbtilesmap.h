#ifndef MBTILESMAP_H
#define MBTILESMAP_H

#include <QDebug>
#include <QSqlDatabase>
#include <QVector>
#include "mvtjob.h"
#include "map.h"

class QPixmap;

class MBTilesMap : public Map
{
	Q_OBJECT

public:
	MBTilesMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi, int style, int layer);
	void unload();

	QStringList styles(int &defaultStyle) const;

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(MVTJob *job);

private:
	struct Zoom {
		Zoom() : z(-1), base(-1)  {}
		Zoom(int z, int base) : z(z), base(base) {}

		int z;
		int base;
	};

	bool getMinZoom(int &zoom);
	bool getMaxZoom(int &zoom);
	bool getZooms();
	bool getBounds();
	bool getTileSizeAndStyle();
	void getTileFormat();
	void getTilePixelRatio();
	void getName();

	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QByteArray tileData(int zoom, const QPoint &tile) const;
	QPointF tilePos(const QPointF &tl, const QPoint &tc, const QPoint &tile,
	  unsigned overzoom) const;
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	QString key(int zoom, const QPoint &xy) const;
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(MVTJob *job);
	void removeJob(MVTJob *job);
	void cancelJobs(bool wait);

	const MVT::Style *defaultStyle() const;

	friend QDebug operator<<(QDebug dbg, const Zoom &zoom);

	QSqlDatabase _db;

	QString _name;
	RectC _bounds;
	QVector<Zoom> _zooms, _zoomsBase;
	int _zoom;
	int _tileSize;
	const MVT::Style *_style;
	qreal _mapRatio, _tileRatio;
	bool _mvt;
	QStringList _layers;

	QList<MVTJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // MBTILESMAP_H
