#ifndef OFFLINEMAP_H
#define OFFLINEMAP_H

#include <QTransform>
#include "map.h"
#include "tar.h"
#include "coordinates.h"

class QIODevice;
class QImage;
class Projection;

class OfflineMap : public Map
{
	Q_OBJECT

public:
	OfflineMap(const QString &path, QObject *parent = 0);
	OfflineMap(Tar &tar, const QString &path, QObject *parent = 0);
	~OfflineMap();

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return 1.0;}
	qreal zoomFit(const QSize &size, const QRectF &br);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	void draw(QPainter *painter, const QRectF &rect);

	void load();
	void unload();

	bool isValid() {return _valid;}

	QPointF ll2pp(const Coordinates &c) const;
	QPointF xy2pp(const QPointF &p) const;
	QPointF pp2xy(const QPointF &p) const;

private:
	typedef struct {
		QPoint xy;
		Coordinates ll;
		QPointF pp;
	} ReferencePoint;

	typedef struct {
		double centralParallel;
		double centralMeridian;
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
	QTransform _transform;
	qreal _resolution;

	Tar _tar;
	QString _tarPath;
	QImage *_img;
	QString _imgPath;
	QSize _tileSize;
	QString _tileName;

	bool _valid;
};

#endif // OFFLINEMAP_H
