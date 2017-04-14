#ifndef OFFLINEMAP_H
#define OFFLINEMAP_H

#include <QTransform>
#include "map.h"
#include "tar.h"
#include "ozf.h"
#include "coordinates.h"
#include "projection.h"


class QIODevice;
class QImage;

class OfflineMap : public Map
{
	Q_OBJECT

public:
	OfflineMap(const QString &path, QObject *parent = 0);
	OfflineMap(Tar &tar, const QString &path, QObject *parent = 0);
	~OfflineMap();

	const QString &name() const {return _name;}

	QRectF bounds() const {return QRectF(QPointF(0, 0), _size);}
	qreal resolution(const QPointF &) const {return _resolution;}

	qreal zoom() const {return 1.0;}
	qreal zoomFit(const QSize &, const QRectF &) {return 1.0;}
	qreal zoomIn() {return 1.0;}
	qreal zoomOut() {return 1.0;}

	QPointF ll2xy(const Coordinates &c) const
	  {return _transform.map(_projection->ll2xy(c));}
	Coordinates xy2ll(const QPointF &p) const
	  {return _projection->xy2ll(_inverted.map(p));}

	void draw(QPainter *painter, const QRectF &rect);

	void load();
	void unload();

	bool isValid() {return _valid;}

	QPointF ll2pp(const Coordinates &c) const
	  {return _projection->ll2xy(c);}
	QPointF xy2pp(const QPointF &p) const
	  {return _inverted.map(p);}
	QPointF pp2xy(const QPointF &p) const
	  {return _transform.map(p);}

private:
	typedef struct {
		QPoint xy;
		Coordinates ll;
		QPointF pp;
	} ReferencePoint;

	typedef struct {
		double latitudeOrigin;
		double longitudeOrigin;
		double scale;
		double falseEasting;
		double falseNorthing;
		double standardParallel1;
		double standardParallel2;
		int zone;
	} ProjectionSetup;

	int parseMapFile(QIODevice &device, QList<ReferencePoint> &points,
	  QString &projection, ProjectionSetup &setup, QString &datum);
	bool mapLoaded(int res);
	bool totalSizeSet();
	bool createProjection(const QString &datum, const QString &projection,
	  const ProjectionSetup &setup, QList<ReferencePoint> &points);
	bool computeTransformation(const QList<ReferencePoint> &points);
	bool computeResolution(QList<ReferencePoint> &points);
	bool getTileInfo(const QStringList &tiles, const QString &path = QString());
	bool getImageInfo(const QString &path);

	QString _name;
	QSize _size;
	Projection *_projection;
	QTransform _transform, _inverted;
	qreal _resolution;

	OZF _ozf;
	Tar _tar;
	QString _tarPath;
	QImage *_img;
	QString _imgPath;
	QSize _tileSize;
	QString _tileName;

	bool _valid;
};

#endif // OFFLINEMAP_H
