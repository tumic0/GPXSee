#ifndef RMAP_H
#define RMAP_H

#include <QFile>
#include <QColor>
#include "map.h"
#include "transform.h"
#include "projection.h"

class RMap : public Map
{
	Q_OBJECT

public:
	RMap(const QString &fileName, QObject *parent = 0);

	QRectF bounds();

	int zoom() const {return _zoom;}
	void setZoom(int zoom) {_zoom = zoom;}
	int zoomFit(const QSize &size, const RectC &rect);
	int zoomIn();
	int zoomOut();

	QPointF ll2xy(const Coordinates &c);
	Coordinates xy2ll(const QPointF &p);

	void setDevicePixelRatio(qreal deviceRatio, qreal mapRatio);
	void load();
	void unload();

	void draw(QPainter *painter, const QRectF &rect, Flags flags);

	bool isValid() const {return _valid;}
	QString errorString() const {return _errorString;}

private:
	struct Header {
		quint32 type;
		quint32 width;
		quint32 height;
		quint32 bpp;
		quint32 unknown;
		quint32 tileWidth;
		quint32 tileHeight;
		quint32 paletteSize;
		quint64 IMPOffset;
	};

	struct Zoom {
		QSize size;
		QSize dim;
		QPointF scale;
		QVector<quint64> tiles;
	};

	bool readHeader(QDataStream &stream, Header &hdr);
	bool readPalette(QDataStream &stream, quint32 paletteSize);
	bool readZooms(QDataStream &stream, const QSize &imageSize);
	bool readZoomLevel(quint64 offset, const QSize &imageSize);
	QByteArray readIMP(quint64 IMPOffset);
	bool parseIMP(const QByteArray &data);
	QPixmap tile(int x, int y);

	QList<Zoom> _zooms;
	Projection _projection;
	Transform _transform;
	QSize _tileSize;
	QFile _file;
	qreal _mapRatio;
	int _zoom;
	QVector<QRgb> _palette;

	bool _valid;
	QString _errorString;
};

#endif // RMAP_H
