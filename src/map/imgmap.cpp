#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtCore>
#else // QT_VERSION < 5
#include <QtConcurrent>
#endif // QT_VERSION < 5
#include "common/rectc.h"
#include "common/wgs84.h"
#include "IMG/textpathitem.h"
#include "IMG/textpointitem.h"
#include "IMG/bitmapline.h"
#include "pcs.h"
#include "rectd.h"
#include "imgmap.h"


#define TILE_SIZE   256
#define TEXT_EXTENT 256

#define LARGE_FONT_SIZE   14
#define NORMAL_FONT_SIZE  12
#define SMALL_FONT_SIZE   10
#define POI_FONT_SIZE      9

#define LINE_TEXT_MIN_ZOOM   23

class RasterTile
{
public:
	RasterTile() : _map(0) {}
	RasterTile(IMGMap *map, const QPoint &xy, const QString &key)
	  : _map(map), _xy(xy), _key(key),
	  _img(TILE_SIZE, TILE_SIZE, QImage::Format_ARGB32_Premultiplied) {}

	const QString &key() const {return _key;}
	const QPoint &xy() const {return _xy;}
	QImage &img() {return _img;}
	QList<IMG::Poly> &polygons() {return _polygons;}
	QList<IMG::Poly> &lines() {return _lines;}
	QList<IMG::Point> &points() {return _points;}

	void load()
	{
		_img.fill(Qt::transparent);

		QPainter painter(&_img);
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.translate(-_xy.x(), -_xy.y());

		_map->drawPolygons(&painter, _polygons);
		_map->drawLines(&painter, _lines, _xy);
		_map->drawPoints(&painter, _points);
	}

private:
	IMGMap *_map;
	QPoint _xy;
	QString _key;
	QImage _img;
	QList<IMG::Poly> _polygons;
	QList<IMG::Poly> _lines;
	QList<IMG::Point> _points;
};


static void convertUnits(QString &str)
{
	bool ok;
	int number = str.toInt(&ok);
	if (ok)
		str = QString::number(qRound(number * 0.3048));
}

static int minPOIZoom(Style::POIClass cl)
{
	switch (cl) {
		case Style::Food:
		case Style::Shopping:
		case Style::Services:
			return 27;
		case Style::Accommodation:
		case Style::Recreation:
			return 25;
		case Style::ManmadePlaces:
		case Style::NaturePlaces:
		case Style::Transport:
		case Style::Community:
		case Style::Elementary:
			return 23;
		default:
			return 0;
	}
}

IMGMap::IMGMap(const QString &fileName, QObject *parent)
  : Map(parent), _fileName(fileName), _img(fileName),
  _projection(PCS::pcs(3857)), _valid(false)
{
	if (!_img.isValid()) {
		_errorString = _img.errorString();
		return;
	}

	_zooms = Range(_img.zooms().min() - 1, 28);
	_zoom = _zooms.min();

	updateTransform();

	_largeFont.setPixelSize(LARGE_FONT_SIZE);
	_normalFont.setPixelSize(NORMAL_FONT_SIZE);
	_smallFont.setPixelSize(SMALL_FONT_SIZE);
	_poiFont.setPixelSize(POI_FONT_SIZE);

	_valid = true;
}

QRectF IMGMap::bounds()
{
	RectD prect(_img.bounds(), _projection);
	return QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
}

int IMGMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		_zoom = _zooms.min();
		for (int i = _zooms.min() + 1; i <= _zooms.max(); i++) {
			Transform t(transform(i));
			QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));
			if (size.width() < r.width() || size.height() < r.height())
				break;
			_zoom = i;
		}
	} else
		_zoom = _zooms.max();

	updateTransform();

	return _zoom;
}

int IMGMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.max());
	updateTransform();
	return _zoom;
}

int IMGMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, _zooms.min());
	updateTransform();
	return _zoom;
}

void IMGMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform IMGMap::transform(int zoom) const
{
	double scale = (2.0 * M_PI * WGS84_RADIUS) / (1<<zoom);
	PointD topLeft(_projection.ll2xy(_img.bounds().topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void IMGMap::updateTransform()
{
	_transform = transform(_zoom);
}

QPointF IMGMap::ll2xy(const Coordinates &c)
{
	return _transform.proj2img(_projection.ll2xy(c));
}

Coordinates IMGMap::xy2ll(const QPointF &p)
{
	return _projection.xy2ll(_transform.img2proj(p));
}


void IMGMap::drawPolygons(QPainter *painter, QList<IMG::Poly> &polygons)
{
	for (int n = 0; n < _img.style().drawOrder().size(); n++) {
		for (int i = 0; i < polygons.size(); i++) {
			IMG::Poly &poly = polygons[i];
			if (poly.type != _img.style().drawOrder().at(n))
				continue;
			const Style::Polygon &style = _img.style().polygon(poly.type);

			for (int j = 0; j < poly.points.size(); j++) {
				QPointF &p = poly.points[j];
				p = ll2xy(Coordinates(p.x(), p.y()));
			}

			painter->setPen(style.pen());
			painter->setBrush(style.brush());
			painter->drawPolygon(poly.points);
		}
	}
}

void IMGMap::drawLines(QPainter *painter, QList<IMG::Poly> &lines,
  const QPoint &tile)
{
	qStableSort(lines);

	for (int i = 0; i < lines.size(); i++) {
		IMG::Poly &poly = lines[i];
		for (int j = 0; j < poly.points.size(); j++) {
			QPointF &p = poly.points[j];
			p = ll2xy(Coordinates(p.x(), p.y()));
		}
	}

	painter->setBrush(Qt::NoBrush);

	for (int i = 0; i < lines.size(); i++) {
		const IMG::Poly &poly = lines.at(i);
		const Style::Line &style = _img.style().line(poly.type);

		if (style.background() == Qt::NoPen)
			continue;

		painter->setPen(style.background());
		painter->drawPolyline(poly.points);
	}

	for (int i = 0; i < lines.size(); i++) {
		const IMG::Poly &poly = lines.at(i);
		const Style::Line &style = _img.style().line(poly.type);

		if (!style.img().isNull())
			BitmapLine::draw(painter, poly.points, style.img());
		else if (style.foreground() != Qt::NoPen) {
			painter->setPen(style.foreground());
			painter->drawPolyline(poly.points);
		}
	}

	if (_zoom < LINE_TEXT_MIN_ZOOM)
		return;

	QVector<TextPathItem> items;

	for (int i = 0; i < lines.size(); i++) {
		IMG::Poly &poly = lines[i];
		const Style::Line &style = _img.style().line(poly.type);

		if (style.img().isNull() && style.foreground() == Qt::NoPen)
			continue;
		if (poly.label.isEmpty() || style.textFontSize() == Style::None)
			continue;

		if (Style::isContourLine(poly.type))
			convertUnits(poly.label);

		const QFont *font;
		switch (style.textFontSize()) {
			case Style::Large:
				font = &_largeFont;
				break;
			case Style::Small:
				font = &_smallFont;
				break;
			default:
				font = &_normalFont;
		}
		const QColor *color = style.textColor().isValid()
		  ? &style.textColor() : 0;

		TextPathItem item(poly.points, &poly.label, QRect(tile,
		  QSize(TILE_SIZE, TILE_SIZE)), font, color);
		if (item.isValid() && !item.collides(items))
			items.append(item);
	}

	for (int i = 0; i < items.size(); i++)
		items.at(i).paint(painter);
}

void IMGMap::drawPoints(QPainter *painter, QList<IMG::Point> &points)
{
	qSort(points);

	QVector<TextPointItem> items;

	for (int i = 0; i < points.size(); i++) {
		IMG::Point &point = points[i];
		const Style::Point &style = _img.style().point(point.type);
		int mz = minPOIZoom(Style::poiClass(point.type));

		if (point.poi && _zoom < mz)
			continue;

		const QString *label = point.label.isEmpty() ? 0 : &(point.label);
		const QImage *img = style.img().isNull() ? 0 : &style.img();
		const QFont *font = 0;
		if (point.poi) {
			if (style.textFontSize() == Style::None)
				label = 0;
			else
				font = &_poiFont;
		} else {
			switch (style.textFontSize()) {
				case Style::None:
					label = 0;
					break;
				case Style::Normal:
					font = &_normalFont;
					break;
				case Style::Small:
					font = &_smallFont;
					break;
				default:
					font = &_largeFont;
			}
		}
		const QColor *color = style.textColor().isValid()
		  ? &style.textColor() : 0;

		if (!label && !img)
			continue;

		if (Style::isSpot(point.type))
			convertUnits(point.label);

		TextPointItem item(ll2xy(point.coordinates).toPoint(), label, font, img,
		  color);
		if (!item.collides(items))
			items.append(item);
	}

	for (int i = 0; i < items.size(); i++)
		items.at(i).paint(painter);
}

static void render(RasterTile &tile)
{
	tile.load();
}

void IMGMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);

	QPointF tl(floor(rect.left() / TILE_SIZE)
	  * TILE_SIZE, floor(rect.top() / TILE_SIZE) * TILE_SIZE);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / TILE_SIZE);
	int height = ceil(s.height() / TILE_SIZE);

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint ttl(tl.x() + i * TILE_SIZE, tl.y() + j * TILE_SIZE);
			QString key = _fileName + "-" + QString::number(_zoom) + "_"
			  + QString::number(ttl.x()) + "_" + QString::number(ttl.y());
			if (QPixmapCache::find(key, pm))
				painter->drawPixmap(ttl, pm);
			else {
				tiles.append(RasterTile(this, ttl, key));
				RasterTile &tile = tiles.last();

				RectD polyRect(_transform.img2proj(ttl), _transform.img2proj(
				  QPointF(ttl.x() + TILE_SIZE, ttl.y() + TILE_SIZE)));
				_img.objects(polyRect.toRectC(_projection, 4), _zoom,
				  &(tile.polygons()), &(tile.lines()), 0);
				RectD pointRect(_transform.img2proj(QPointF(ttl.x() - TEXT_EXTENT,
				  ttl.y() - TEXT_EXTENT)), _transform.img2proj(QPointF(ttl.x()
				  + TILE_SIZE + TEXT_EXTENT, ttl.y() + TILE_SIZE + TEXT_EXTENT)));
				_img.objects(pointRect.toRectC(_projection, 4), _zoom,
				  0, 0, &(tile.points()));
			}
		}
	}

	QFuture<void> future = QtConcurrent::map(tiles, render);
	future.waitForFinished();

	for (int i = 0; i < tiles.size(); i++) {
		RasterTile &mt = tiles[i];
		QPixmap pm(QPixmap::fromImage(mt.img()));
		if (pm.isNull())
			continue;

		QPixmapCache::insert(mt.key(), pm);

		painter->drawPixmap(mt.xy(), pm);
	}
}

void IMGMap::setProjection(const Projection &projection)
{
	_projection = projection;
	updateTransform();
	QPixmapCache::clear();
}
