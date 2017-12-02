#ifndef ATLAS_H
#define ATLAS_H

#include <QFileInfoList>
#include "map.h"
#include "offlinemap.h"


class Atlas : public Map
{
	Q_OBJECT

public:
	Atlas(const QString &fileName, QObject *parent = 0);
	~Atlas();

	const QString &name() const {return _name;}

	QRectF bounds() const;
	qreal resolution(const QPointF &p) const;

	qreal zoom() const;
	qreal zoomFit(const QSize &size, const RectC &br);
	qreal zoomFit(qreal resolution, const Coordinates &c);
	qreal zoomIn();
	qreal zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect);

	void unload();

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

private:
	void draw(QPainter *painter, const QRectF &rect, int mapIndex);
	bool isAtlas(Tar &tar, const QString &path);
	void computeZooms();
	void computeBounds();

	QString _name;
	bool _valid;
	QString _errorString;

	QList<OfflineMap*> _maps;
	QVector<QPair<int, int> > _zooms;
	QVector<QPair<QRectF, QRectF> > _bounds;
	int _zoom;

	int _ci, _cz;
};

#endif // ATLAS_H
