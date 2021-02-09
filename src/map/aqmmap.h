#ifndef AQMMAP_H
#define AQMMAP_H

#include <QDebug>
#include <QFile>
#include <QHash>
#include "common/config.h"
#include "map.h"

class AQMMap : public Map
{
public:
	Q_OBJECT

public:
	AQMMap(const QString &fileName, QObject *parent = 0);

	QString name() const {return _name;}

	QRectF bounds();
	RectC llBounds() {return _bounds;}
	qreal resolution(const QRectF &rect);

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	void load();
	void unload();
	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	struct File {
		QByteArray name;
		size_t offset;
	};

	struct Zoom {
		Zoom() : zoom(-1), tileSize(-1) {}
		Zoom(int zoom, int tileSize) : zoom(zoom), tileSize(tileSize) {}

		int zoom;
		int tileSize;
		QHash<QPoint, size_t> tiles;
	};

	bool readSize(size_t &size);
	bool readString(QByteArray &str);
	bool readFile(File &file);
	bool readData(QByteArray &data);
	bool readHeader();

	qreal tileSize() const;
	QByteArray tileData(const QPoint &tile);
	void drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp);

	friend QDebug operator<<(QDebug dbg, const File &file);
	friend QDebug operator<<(QDebug dbg, const Zoom &zoom);

	QString _name;
	QFile _file;
	QVector<Zoom> _zooms;
	int _zoom;
	RectC _bounds;
	qreal _mapRatio;

	bool _valid;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const AQMMap::File &file);
QDebug operator<<(QDebug dbg, const AQMMap::Zoom &zoom);
#endif // QT_NO_DEBUG

#endif // AQMMAP_H
