#include "common/util.h"
#include "map/dem.h"
#include "map/hillshading.h"
#include "map/filter.h"
#include "map/osm.h"
#include "data.h"
#include "rastertile.h"

using namespace MVT;

RasterTile::RasterTile(const QByteArray &data, bool mvt, bool gzip,
  const Style *style, int zoom, const QPoint &xy, int size, qreal ratio,
  int overzoom, bool hillShading) : _data(data), _mvt(mvt), _gzip(gzip),
  _style(style), _zoom(zoom), _xy(xy), _ratio(ratio), _hillShading(hillShading)
{
	_size = qMin(size<<overzoom, 4096);
}

void RasterTile::render()
{
	QByteArray rawData(_gzip ? Util::gunzip(_data) : _data);

	if (_mvt) {
		QImage img(_size * _ratio, _size * _ratio,
		  QImage::Format_ARGB32_Premultiplied);
		img.setDevicePixelRatio(_ratio);
		img.fill(Qt::transparent);

		if (_style)
			renderMVT(rawData, &img);

		_pixmap.convertFromImage(img);
	} else
		_pixmap.loadFromData(rawData);
}

static Tile::Layer *tileLayer(const Tile &tile, const Style::Layer &sl)
{
	QHash<QByteArray, Tile::Layer*>::const_iterator it =
	  tile.layers().find(sl.sourceLayer());
	return (it == tile.layers().constEnd()) ? 0 : *it;
}

void RasterTile::renderMVT(const QByteArray &rawData, QImage *img)
{
	Data data(rawData);
	Tile tile(data);
	Text text(_zoom, _size, _ratio, _style);
	QPainter painter(img);

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);

	for (int i = 0; i < _style->layers().size(); i++) {
		const Style::Layer &sl = _style->layers().at(i);
		if (!sl.isVisible())
			continue;

		switch (sl.type()) {
			case Style::Layer::Background:
				drawBackground(painter, sl);
				break;
			case Style::Layer::Hillshade:
				drawHillshading(painter, sl);
				break;
			case Style::Layer::Line:
			case Style::Layer::Fill:
				drawLayer(painter, sl, tileLayer(tile, sl));
				break;
			case Style::Layer::Symbol:
				text.addLayer(sl, tileLayer(tile, sl));
				break;
			default:
				break;
		}
	}

	text.render(&painter);

	//QRectF rect(QPointF(0, 0), QSizeF(_scaledSize, _scaledSize));
	//painter.setPen(Qt::red);
	//painter.setBrush(Qt::NoBrush);
	//painter.setRenderHint(QPainter::Antialiasing, false);
	//painter.drawRect(rect);
}

void RasterTile::drawBackground(QPainter &painter,
  const Style::Layer &styleLayer)
{
	QRectF rect(QPointF(0, 0), QSizeF(_size, _size));
	QPainterPath path;
	path.addRect(rect);

	styleLayer.setPathPainter(_zoom, _style->sprites(_ratio), painter);
	painter.drawPath(path);
}

void RasterTile::drawFeature(QPainter &painter, const Style::Layer &layer,
  Tile::Feature &feature)
{
	if (layer.match(_zoom, feature))
		painter.drawPath(feature.path(_size));
}

void RasterTile::drawLayer(QPainter &painter, const Style::Layer &styleLayer,
  Tile::Layer *pbfLayer)
{
	if (pbfLayer) {
		painter.save();
		styleLayer.setPathPainter(_zoom, _style->sprites(_ratio), painter);
		for (int i = 0; i < pbfLayer->features().size(); i++)
			drawFeature(painter, styleLayer, pbfLayer->features()[i]);
		painter.restore();
	}
}

void RasterTile::drawHillshading(QPainter &painter,
  const Style::Layer &styleLayer)
{
	if (_hillShading && styleLayer.match(_zoom)) {
		MatrixD ele(elevation(HillShading::blur() + 1));
		if (ele.isNull())
			return;

		if (HillShading::blur()) {
			MatrixD dem(Filter::blur(ele, HillShading::blur()));
			QImage img(HillShading::render(dem, HillShading::blur() + 1));
			painter.drawImage(0, 0, img);
		} else
			painter.drawImage(0, 0, HillShading::render(ele, 1));
	}
}

static inline Coordinates xy2ll(int x, int y, qreal factor)
{
	return OSM::m2ll(QPointF(x * factor, -y * factor));
}

MatrixD RasterTile::elevation(int extend) const
{
	qreal scale = OSM::zoom2scale(_zoom, _size * _ratio);
	qreal factor = scale * _ratio;
	QPointF tlm(OSM::tile2mercator(_xy, _zoom));
	QPointF tl(QPointF(tlm.x() / scale, tlm.y() / scale) / _ratio);

	int left = (int)tl.x() - extend;
	int right = (int)(tl.x() + _size * _ratio) + extend;
	int top = (int)tl.y() - extend;
	int bottom = (int)(tl.y() + _size * _ratio) + extend;

	RectC rect(xy2ll(left, top, factor), xy2ll(right, bottom, factor));
	if (!DEM::elevation(rect))
		return MatrixD();

	MatrixC ll((int)(_size * _ratio) + 2 * extend,
	  (int)(_size * _ratio) + 2 * extend);
	for (int y = top, i = 0; y < bottom; y++)
		for (int x = left; x < right; x++, i++)
			ll.at(i) = xy2ll(x, y, factor);

	return DEM::elevation(ll);
}
