#ifndef ATLAS_H
#define ATLAS_H

#include "map.h"

class OfflineMap;

class Atlas : public Map
{
	Q_OBJECT

public:
	Atlas(const QString &fileName, QObject *parent = 0);

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QRectF &rect) const;

	int zoom() const {return _zoom;}
	int zoomFit(const QSize &size, const RectC &br);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect);

	void unload();

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

	static bool isAtlas(const QString &path);

private:
	void draw(QPainter *painter, const QRectF &rect, int mapIndex);
	void computeZooms();
	void computeBounds();

	QString _name;

	QList<OfflineMap*> _maps;
	QVector<QPair<int, int> > _zooms;
	QVector<QPair<QRectF, QRectF> > _bounds;
	int _zoom;
	int _mapIndex;

	bool _valid;
	QString _errorString;
};

#endif // ATLAS_H
