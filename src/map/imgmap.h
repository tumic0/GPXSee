#ifndef IMGMAP_H
#define IMGMAP_H

#include "map.h"
#include "projection.h"
#include "transform.h"
#include "IMG/mapdata.h"

class IMGJob;

class IMGMap : public Map
{
	Q_OBJECT

public:
	IMGMap(const QString &fileName, bool GMAP, QObject *parent = 0);
	~IMGMap();

	QString name() const {return _data.first()->name();}

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _data.first()->bounds();}

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

	static Map* createIMG(const QString &path, const Projection &proj,
	  bool *isDir);
	static Map* createGMAP(const QString &path, const Projection &proj,
	  bool *isDir);

private slots:
	void jobFinished(IMGJob *job);

private:
	enum Layer {
		Vector = 1,
		Raster = 2,
		All = 3
	};

	class StyleList : public QStringList
	{
	public:
		StyleList();
	};

	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(const QString &key) const;
	void runJob(IMGJob *job);
	void removeJob(IMGJob *job);
	void cancelJobs(bool wait);

	QList<IMG::MapData*> overlays(const QString &fileName);
	IMG::Style *createStyle(IMG::MapData *data, const QString *typFile);

	static StyleList &styles();

	QList<IMG::MapData*> _data;
	QList<IMG::Style*> _styles;
	IMG::MapData::PolyCache _polyCache;
	IMG::MapData::PointCache _pointCache;
	IMG::MapData::ElevationCache _demCache;
	QMutex _lock, _demLock;
	int _zoom;
	Projection _projection;
	Transform _transform;
	QRectF _bounds;
	RectC _dataBounds;
	qreal _tileRatio;
	Layer _layer;

	QList<IMGJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // IMGMAP_H
