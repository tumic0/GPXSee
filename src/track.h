#ifndef TRACK_H
#define TRACK_H

#include <QGraphicsView>
#include <QVector>
#include <QHash>
#include <QList>
#include <QPrinter>
#include "poi.h"
#include "gpx.h"
#include "colorshop.h"


class POIItem;
class MarkerItem;

class Track : public QGraphicsView
{
	Q_OBJECT

public:
	Track(QWidget *parent = 0);
	~Track();

	void loadGPX(const GPX &gpx);
	void loadPOI(const POI &poi);

	void clearPOI();
	void clear();

	void plot(QPainter *painter, const QRectF &target);
	enum QPrinter::Orientation orientation() const;

public slots:
	void movePositionMarker(qreal val);

private:
	QTransform transform() const;
	void wheelEvent(QWheelEvent *event);

	QGraphicsScene *_scene;
	QList<QVector<QPointF> > _tracks;
	QList<QGraphicsPathItem*> _trackPaths;
	QList<MarkerItem*> _markers;
	QHash<Entry, POIItem*> _pois;

	ColorShop _colorShop;
	qreal _maxLen;
};

#endif // TRACK_H
