#ifndef MVT_RASTERTILE_H
#define MVT_RASTERTILE_H

#include <QImage>
#include <QPainter>
#include "map/matrix.h"
#include "text.h"
#include "style.h"

namespace MVT {

class RasterTile
{
public:
	RasterTile(const QByteArray &data, bool mvt, bool gzip, const Style *style,
	  int zoom, const QPoint &xy, int size, qreal ratio, int overzoom,
	  bool hillShading);

	int zoom() const {return _zoom;}
	QPoint xy() const {return _xy;}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	QByteArray _data;
	bool _mvt, _gzip;
	const Style *_style;
	int _zoom;
	QPoint _xy;
	qreal _ratio;
	int _size;
	bool _hillShading;
	QPixmap _pixmap;

	void renderMVT(const QByteArray &rawData, QImage *img);
	void drawBackground(QPainter &painter, const Style::Layer &styleLayer);
	void drawFeature(QPainter &painter, const Style::Layer &layer,
	  Tile::Feature &feature);
	void drawLayer(QPainter &painter, const Style::Layer &styleLayer,
	  Tile::Layer *pbfLayer);
	void drawHillshading(QPainter &painter, const Style::Layer &styleLayer);
	MatrixD elevation(int extend) const;
	Coordinates xy2ll(const QPointF &p, qreal scale) const;
};

}

#endif // MVT_RASTERTILE_H
