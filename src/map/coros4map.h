#ifndef COROS4MAP_H
#define COROS4MAP_H

#include "map.h"
#include "projection.h"
#include "transform.h"
#include "IMG/mapdata.h"

class IMGJob;
namespace IMG {class Style;}

class Coros4Map : public Map
{
	Q_OBJECT

public:
	Coros4Map(const QString &fileName, QObject *parent = 0);
	~Coros4Map();

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _dataBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &, const RectC &);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal devicelRatio,
	  bool hidpi, int style, int layer);
	void unload();

	double elevation(const Coordinates &c);

	QStringList styles(int &defaultStyle) const;
	QStringList layers(const QString &lang, int &defaultLayer) const;
	bool hillShading() const {return true;}

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map* create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(IMGJob *job);

private:
	enum Layer {
		Landscape = 1,
		Topo = 2,
		All = 3
	};

	class StyleList : public QStringList
	{
	public:
		StyleList();
	};

	typedef RTree<IMG::MapData*, double, 2> MapTree;

	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(const QString &key) const;
	void runJob(IMGJob *job);
	void removeJob(IMGJob *job);
	void cancelJobs(bool wait);

	void loadDir(const QString &path, MapTree &tree);

	static StyleList &styles();

	MapTree _osm, _cm;
	Range _zooms;
	int _zoom;
	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	RectC _dataBounds;
	qreal _tileRatio;
	Layer _layer;
	IMG::Style *_style;
	IMG::MapData::PolyCache _polyCache;
	IMG::MapData::PointCache _pointCache;
	IMG::MapData::ElevationCache _demCache;
	QMutex _lock, _demLock;
	QString _typ;

	QList<IMGJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // COROS4MAP_H
