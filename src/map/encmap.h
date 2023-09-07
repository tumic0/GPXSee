#ifndef ENCMAP_H
#define ENCMAP_H

#include <climits>
#include <QtConcurrent>
#include "common/range.h"
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "ENC/mapdata.h"
#include "ENC/iso8211.h"

class ENCJob;

class ENCMap : public Map
{
	Q_OBJECT

public:
	ENCMap(const QString &fileName, QObject *parent = 0);
	~ENCMap() {delete _data;}

	QString name() const {return _name;}

	QRectF bounds() {return _bounds;}
	RectC llBounds(const Projection &) {return _llBounds;}

	int zoom() const {return _zoom;}
	void setZoom(int zoom);
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	void load(const Projection &in, const Projection &out, qreal deviceRatio,
	  bool hidpi);
	void unload();

	QPointF ll2xy(const Coordinates &c)
	  {return _transform.proj2img(_projection.ll2xy(c));}
	Coordinates xy2ll(const QPointF &p)
	  {return _projection.xy2ll(_transform.img2proj(p));}

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, bool *isMap);

private slots:
	void jobFinished(ENCJob *job);

private:
	class Rect {
	public:
		Rect()
		  : _minX(INT_MAX), _maxX(INT_MIN), _minY(INT_MAX), _maxY(INT_MIN) {}
		Rect(int minX, int maxX, int minY, int maxY)
		  : _minX(minX), _maxX(maxX), _minY(minY), _maxY(maxY) {}

		int minX() const {return _minX;}
		int maxX() const {return _maxX;}
		int minY() const {return _minY;}
		int maxY() const {return _maxY;}

		void unite(int x, int y) {
			if (x < _minX)
				_minX = x;
			if (x > _maxX)
				_maxX = x;
			if (y < _minY)
				_minY = y;
			if (y > _maxY)
				_maxY = y;
		}

		Rect &operator|=(const Rect &r) {*this = *this | r; return *this;}
		Rect operator|(const Rect &r) const
		{
			return Rect(qMin(_minX, r._minX), qMax(_maxX, r._maxX),
			  qMin(_minY, r._minY), qMax(_maxY, r._maxY));
		}

	private:
		int _minX, _maxX, _minY, _maxY;
	};

	Transform transform(int zoom) const;
	void updateTransform();
	bool isRunning(int zoom, const QPoint &xy) const;
	void runJob(ENCJob *job);
	void removeJob(ENCJob *job);
	void cancelJobs(bool wait);
	QString key(int zoom, const QPoint &xy) const;

	static bool bounds(const ENC::ISO8211::Record &record, Rect &rect);
	static bool bounds(const QVector<ENC::ISO8211::Record> &gv, Rect &b);
	static bool processRecord(const ENC::ISO8211::Record &record,
	  QVector<ENC::ISO8211::Record> &rv, uint &COMF, QString &name);

	QString _name;
	ENC::MapData *_data;
	Projection _projection;
	Transform _transform;
	qreal _tileRatio;
	RectC _llBounds;
	QRectF _bounds;
	Range _zooms;
	int _zoom;

	QList<ENCJob*> _jobs;

	bool _valid;
	QString _errorString;
};

#endif // ENCMAP_H
