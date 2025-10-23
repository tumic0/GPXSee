#ifndef MVT_RASTERTILE_H
#define MVT_RASTERTILE_H

#include <QImage>
#include <QPainter>
#include "text.h"
#include "style.h"

namespace MVT {

class RasterTile
{
public:
	RasterTile(const QByteArray &data, bool mvt, bool gzip, const Style *style,
	  int zoom, const QRect &rect, qreal ratio, int overzoom);

	int zoom() const {return _zoom;}
	QPoint xy() const {return _rect.topLeft();}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	QByteArray _data;
	bool _mvt, _gzip;
	const Style *_style;
	int _zoom;
	QRect _rect;
	qreal _ratio;
	QPixmap _pixmap;
	int _size, _scaledSize;

	void renderMVT(const QByteArray &rawData, QImage *img);
	void drawBackground(QPainter &painter, const Style::Layer &styleLayer);
	void drawFeature(QPainter &painter, const Style::Layer &layer,
	  Tile::Feature &feature);
	void drawLayer(QPainter &painter, const Style::Layer &styleLayer,
	  Tile::Layer &pbfLayer);
};

}

#endif // MVT_RASTERTILE_H
