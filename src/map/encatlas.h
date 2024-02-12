#ifndef ENCATLAS_H
#define ENCATLAS_H

#include <QMap>
#include <QMutex>
#include "common/range.h"
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "ENC/iso8211.h"
#include "ENC/atlasdata.h"
#include "ENC/style.h"

class ENCJob;
class QDir;

class ENCAtlas : public Map
{
	Q_OBJECT

public:
	ENCAtlas(const QString &fileName, QObject *parent = 0);
	~ENCAtlas();

	QString name() const {return _name;}

	QRectF bounds() {return _bounds;}
	RectC llBounds() {return _llBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &proj, bool *isDir);

private slots:
	void jobFinished(ENCJob *job);

private:
	enum IntendedUsage {
		Unknown = 0,
		Overview = 1,
		General = 2,
		Coastal = 3,
		Approach = 4,
		Harbour = 5,
		Berthing = 6,
		River = 7,
		RiverHarbour = 8,
		RiverBerthing = 9
	};

	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(ENCJob *job);
	void removeJob(ENCJob *job);
	void cancelJobs(bool wait);
	QString key(int zoom, const QPoint &xy) const;
	void addMap(const QDir &dir, const QByteArray &file, const RectC &bounds);

	static bool processRecord(const ENC::ISO8211::Record &record,
	  QByteArray &file, RectC &bounds);
	static Range zooms(IntendedUsage usage);
	static IntendedUsage usage(const QString &path);

	QString _name;
	RectC _llBounds;
	QRectF _bounds;
	Projection _projection;
	Transform _transform;
	qreal _tileRatio;
	QMap<IntendedUsage, ENC::AtlasData*> _data;
	ENC::Style *_style;
	ENC::MapCache _cache;
	QMutex _lock;
	IntendedUsage _usage;
	int _zoom;

	QList<ENCJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // ENCATLAS_H
