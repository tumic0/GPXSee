#ifndef PMTILESMAP_H
#define PMTILESMAP_H

#include <QFile>
#include "pmtiles.h"
#include "mvtjob.h"
#include "map.h"

class PMTilesMap : public Map
{
public:
	Q_OBJECT

public:
	PMTilesMap(const QString &fileName, QObject *parent = 0);

	QString name() const;

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
	bool hillShading() const;

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

	QPointF tilePos(const QPointF &tl, const QPoint &tc, const QPoint &tile,
	  unsigned overzoom) const;
	qreal tileSize() const;
	qreal coordinatesRatio() const;
	qreal imageRatio() const;
	QByteArray tileData(quint64 id);
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	QString key(int zoom, const QPoint &xy) const;
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(MVTJob *job);
	void removeJob(MVTJob *job);
	void cancelJobs(bool wait);

	const MVT::Style *defaultStyle() const;

	QFile _file;
	QString _name;
	RectC _bounds;
	QVector<PMTiles::Directory> _root;
	QCache<quint64, QVector<PMTiles::Directory> > _cache;
	quint64 _tileOffset, _leafOffset;
	quint8 _tc, _ic;
	QVector<Zoom> _zooms, _zoomsBase;
	const MVT::Style *_style;
	int _zoom;
	int _tileSize;
	qreal _mapRatio, _tileRatio;
	bool _mvt;
	QStringList _layers;

	QList<MVTJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // PMTILESMAP_H
