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
	OfflineMap(const QString &fileName, QObject *parent = 0);
	OfflineMap(const QString &fileName, Tar &tar, QObject *parent = 0);
	~OfflineMap();

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const {return _zoom;}
	qreal zoomFit(const QSize &size, const RectC &br);
	qreal zoomFit(qreal resolution, const Coordinates &c);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect);

	void load();
	void unload();

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

	QPointF ll2pp(const Coordinates &c) const
	  {return _projection->ll2xy(c);}
	QPointF xy2pp(const QPointF &p) const
	  {return _inverted.map(p);}
	QPointF pp2xy(const QPointF &p) const
	  {return _transform.map(p);}

private:
	struct ReferencePoint {
		QPoint xy;
		Coordinates ll;
		QPointF pp;
	};

	struct ProjectionSetup {
		double latitudeOrigin;
		double longitudeOrigin;
		double scale;
		double falseEasting;
		double falseNorthing;
		double standardParallel1;
		double standardParallel2;
		int zone;
	};

	int parse(QIODevice &device, QList<ReferencePoint> &points,
	  QString &projection, ProjectionSetup &setup, QString &datum);
	bool parseMapFile(QIODevice &device, QList<ReferencePoint> &points,
	  QString &projection, ProjectionSetup &setup, QString &datum);
	bool totalSizeSet();
	bool createProjection(const QString &datum, const QString &projection,
	  const ProjectionSetup &setup, QList<ReferencePoint> &points);
	bool simpleTransformation(const QList<ReferencePoint> &points);
	bool affineTransformation(const QList<ReferencePoint> &points);
	bool computeTransformation(const QList<ReferencePoint> &points);
	bool computeResolution(QList<ReferencePoint> &points);
	bool getTileInfo(const QStringList &tiles, const QString &path = QString());
	bool getImageInfo(const QString &path);

	void drawTiled(QPainter *painter, const QRectF &rect);
	void drawOZF(QPainter *painter, const QRectF &rect);
	void drawImage(QPainter *painter, const QRectF &rect);

	void rescale(int zoom);

	QString _name;
	bool _valid;
	QString _errorString;

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

	int _zoom;
	QPointF _scale;
};

#endif // OFFLINEMAP_H
