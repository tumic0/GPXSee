#include "common/util.h"
#include "data.h"
#include "rastertile.h"

using namespace MVT;

RasterTile::RasterTile(const QByteArray &data, bool mvt, bool gzip,
  const Style *style, int zoom, const QRect &rect, qreal ratio, int overzoom)
  : _data(data), _mvt(mvt), _gzip(gzip), _style(style), _zoom(zoom),
  _rect(rect), _ratio(ratio)
{
	_size = qMin(rect.width()<<overzoom, 4096);
	_scaledSize = _size / _ratio;
}

void RasterTile::render()
{
	if (_mvt) {
		QImage img(_size, _size, QImage::Format_ARGB32_Premultiplied);
		img.fill(Qt::transparent);

		if (_style)
			renderMVT(&img);

		_pixmap.convertFromImage(img);
	} else
		_pixmap.loadFromData(_data);
}

void RasterTile::renderMVT(QImage *img)
{
	QByteArray rawData(_gzip ? Util::gunzip(_data) : _data);
	Data data(rawData);
	Tile pbf(data);
	Text text(_zoom, _scaledSize, _ratio, _style);
	QPainter painter(img);

	painter.scale(_ratio, _ratio);

	for (int i = 0; i < _style->layers().size(); i++) {
		const Style::Layer &sl = _style->layers().at(i);
		if (!sl.isVisible())
			continue;

		if (sl.isBackground())
			drawBackground(painter, sl);
		else {
			QHash<QByteArray, Tile::Layer*>::const_iterator it =
			  pbf.layers().find(sl.sourceLayer());
			if (it == pbf.layers().constEnd())
				continue;

			if (sl.isPath())
				drawLayer(painter, sl, **it);
			else if (sl.isSymbol())
				text.addLayer(&sl, *it);
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
	QRectF rect(QPointF(0, 0), QSizeF(_scaledSize, _scaledSize));
	QPainterPath path;
	path.addRect(rect);

	styleLayer.setPathPainter(_zoom, _style->sprites(_ratio), painter);
	painter.drawPath(path);
}

void RasterTile::drawFeature(QPainter &painter, const Style::Layer &layer,
  Tile::Feature &feature)
{
	if (layer.match(_zoom, feature))
		painter.drawPath(feature.path(_scaledSize));
}

void RasterTile::drawLayer(QPainter &painter, const Style::Layer &styleLayer,
  Tile::Layer &pbfLayer)
{
	painter.save();
	styleLayer.setPathPainter(_zoom, _style->sprites(_ratio), painter);
	for (int i = 0; i < pbfLayer.features().size(); i++)
		drawFeature(painter, styleLayer, pbfLayer.features()[i]);
	painter.restore();
}
