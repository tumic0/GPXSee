#ifndef ORUXMAP_H
#define ORUXMAP_H

#include <QDebug>
#include <QSqlDatabase>
#include <QDir>
#include "map.h"
#include "projection.h"
#include "transform.h"
#include "calibrationpoint.h"

class QXmlStreamReader;

class OruxMap : public Map
{
	Q_OBJECT

public:
	OruxMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);
	void load();
	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static Map *create(const QString &path, const Projection &, bool *isDir);

private:
	struct Zoom {
		Zoom(int zoom, const QSize &tileSize, const QSize &size,
		  const Projection &proj, const Transform &transform,
		  const QString &fileName, const QString &set)
		  : zoom(zoom), tileSize(tileSize), size(size), projection(proj),
		  transform(transform), fileName(fileName), set(set) {}
		bool operator<(const Zoom &other) const
		  {return zoom < other.zoom;}

		int zoom;
		QSize tileSize;
		QSize size;
		Projection projection;
		Transform transform;
		QString fileName;
		QDir set;
	};

	bool readXML(const QString &path, const QString &dir = QString());
	void oruxTracker(QXmlStreamReader &reader, const QString &dir, int level);
	void mapCalibration(QXmlStreamReader &reader, const QString &dir, int level);
	void calibrationPoints(QXmlStreamReader &reader, const QSize &size,
	  QList<CalibrationPoint> &points);
	QPixmap tile(const Zoom &z, int x, int y) const;

	friend QDebug operator<<(QDebug dbg, const Zoom &zoom);

	QString _name;
	QList<Zoom> _zooms;
	QSqlDatabase _db;
	int _zoom;
	qreal _mapRatio;

	bool _valid;
	QString _errorString;
};

#endif // ORUXMAP_H
