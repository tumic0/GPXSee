#ifndef MVT_RASTERTILE_H
#define MVT_RASTERTILE_H

#include <QImage>
#include <QPainter>
#include "map/matrix.h"
#include "text.h"
#include "style.h"
#include "source.h"

namespace MVT {

class RasterTile
{
public:
	RasterTile(const Source &data, const Style *style,
	  int zoom, const QPoint &xy, int size, qreal ratio, int overzoom,
	  bool hillShading);
	RasterTile(const QList<Source> &data, const Style *style,
	  int zoom, const QPoint &xy, int size, qreal ratio, int overzoom,
	  bool hillShading);

	int zoom() const {return _zoom;}
	QPoint xy() const {return _xy;}
	const QPixmap &pixmap() const {return _pixmap;}

	void render();

private:
	QList<Source> _data;
	const Style *_style;
	int _zoom;
	QPoint _xy;
	qreal _ratio;
	int _size;
	bool _hillShading;
	QPixmap _pixmap;

	void renderMVT(QPainter &painter, const VectorTile &tile);
	void drawBackground(QPainter &painter, const Style::Layer &styleLayer);
	void drawFeature(QPainter &painter, const Style::Layer &layer,
	  VectorTile::Feature &feature);
	void drawLayer(QPainter &painter, const Style::Layer &styleLayer,
	  VectorTile::Layer *pbfLayer);
	void drawHillshading(QPainter &painter, const Style::Layer &styleLayer);
	MatrixD elevation(int extend) const;
};

}

#endif // MVT_RASTERTILE_H
