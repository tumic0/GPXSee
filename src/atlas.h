#ifndef ATLAS_H
#define ATLAS_H

#include <QFileInfoList>
#include "map.h"
#include "offlinemap.h"


class Atlas : public Map
{
	Q_OBJECT

public:
	Atlas(const QString &path, QObject *parent = 0);
	~Atlas();

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const;
	qreal zoomFit(const QSize &size, const QRectF &br);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c) const;
	Coordinates xy2ll(const QPointF &p) const;

	void draw(QPainter *painter, const QRectF &rect);

	bool isValid() {return _valid;}

private:
	void draw(QPainter *painter, const QRectF &rect, int mapIndex);
	bool isAtlas(const QFileInfoList &files);
	void computeZooms();
	void computeBounds();

	QString _name;
	bool _valid;

	Tar _tar;
	QList<OfflineMap*> _maps;
	QVector<QPair<int, int> > _zooms;
	QVector<QPair<QRectF, QRectF> > _bounds;
	int _zoom;
};

#endif // ATLAS_H
