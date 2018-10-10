#include <QtEndian>
#include <QPainter>
#include <QFileInfo>
#include <QPixmapCache>
#include "rectd.h"
#include "gcs.h"
#include "pcs.h"
#include "config.h"
#include "jnxmap.h"


#define ic2dc(x) ((x) * 180.0 / 0x7FFFFFFF)

struct Level {
	quint32 count;
	quint32 offset;
	quint32 scale;
};

struct Ctx {
	QPainter *painter;
	QFile *file;
	qreal ratio;

	Ctx(QPainter *painter, QFile *file, qreal ratio)
	  : painter(painter), file(file), ratio(ratio) {}
};


template<class T> bool JNXMap::readValue(T &val)
{
	T data;

	if (_file.read((char*)&data, sizeof(T)) < (qint64)sizeof(T))
		return false;

	if (sizeof(T) > 1)
		val = qFromLittleEndian(data);
	else
		val = data;

	return true;
}

bool JNXMap::readString(QByteArray& ba)
{
	char byte;

	while (true) {
		if (!_file.getChar(&byte))
			return false;
		else if (!byte)
			return true;
		else
			ba += byte;
	}
}

bool JNXMap::readTiles()
{
	qint32 lat1, lon2, lat2, lon1;
	quint32 version, dummy, levels;

	if (!(readValue(version) && readValue(dummy) && readValue(lat1)
	  && readValue(lon2) && readValue(lat2) && readValue(lon1)
	  && readValue(levels)))
		return false;

	_bounds = RectC(Coordinates(ic2dc(lon1), ic2dc(lat1)),
	  Coordinates(ic2dc(lon2), ic2dc(lat2)));
	if (!levels || !_bounds.isValid())
		return false;

	if (!_file.seek(version > 3 ? 0x34 : 0x30))
		return false;

	QVector<Level> lh(levels);
	for (int i = 0; i < lh.count(); i++) {
		Level &l = lh[i];

		if (!(readValue(l.count) && readValue(l.offset) && readValue(l.scale)))
			return false;
		if (version > 3) {
			QByteArray ba;
			if (!(readValue(dummy) && readString(ba)))
				return false;
		}
	}

	QByteArray guid;
	if (!(readValue(dummy) && readString(guid)))
		return false;
	/* Use WebMercator projection for nakarte.tk maps */
	if (guid == "12345678-1234-1234-1234-123456789ABC")
		_projection = Projection(PCS::pcs(3857));
	else
		_projection = Projection(GCS::gcs(4326));

	_zooms = QVector<Zoom>(lh.size());
	for (int i = 0; i < lh.count(); i++) {
		Zoom &z = _zooms[i];
		const Level &l = lh.at(i);

		if (!_file.seek(l.offset))
			return false;

		z.tiles = QVector<Tile>(l.count);
		for (quint32 j = 0; j < l.count; j++) {
			Tile &tile = z.tiles[j];
			qint32 top, right, bottom, left;
			quint16 width, height;

			if (!(readValue(top) && readValue(right) && readValue(bottom)
			  && readValue(left) && readValue(width)
			  && readValue(height) && readValue(tile.size)
			  && readValue(tile.offset)))
				return false;

			RectD rect(_projection.ll2xy(Coordinates(ic2dc(left), ic2dc(top))),
			  _projection.ll2xy(Coordinates(ic2dc(right), ic2dc(bottom))));

			if (j == 0) {
				ReferencePoint tl(PointD(0, 0), rect.topLeft());
				ReferencePoint br(PointD(width, height), rect.bottomRight());
				z.transform = Transform(tl, br);
			}

			QRectF trect(z.transform.proj2img(rect.topLeft()),
			  z.transform.proj2img(rect.bottomRight()));
			tile.pos = trect.topLeft();

			qreal min[2], max[2];
			min[0] = trect.left();
			min[1] = trect.top();
			max[0] = trect.right();
			max[1] = trect.bottom();
			z.tree.Insert(min, max, &tile);
		}
	}

	return true;
}

JNXMap::JNXMap(const QString &fileName, QObject *parent)
  : Map(parent), _file(fileName), _zoom(0), _ratio(1.0), _valid(false)
{
	_name = QFileInfo(fileName).fileName();

	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = QString("%1: Error opening file").arg(fileName);
		return;
	}

	if (!readTiles()) {
		_errorString = "JNX file format error";
		return;
	}

	_valid = true;
}

QPointF JNXMap::ll2xy(const Coordinates &c)
{
	const Zoom &z = _zooms.at(_zoom);
	return z.transform.proj2img(_projection.ll2xy(c)) / _ratio;
}

Coordinates JNXMap::xy2ll(const QPointF &p)
{
	const Zoom &z = _zooms.at(_zoom);
	return _projection.xy2ll(z.transform.img2proj(p * _ratio));
}

QRectF JNXMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int JNXMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zoom = _zooms.size() - 1;
	else {
		for (int i = 1; i < _zooms.count(); i++) {
			_zoom = i;
			QRect sbr(QPoint(ll2xy(rect.topLeft()).toPoint()),
			  QPoint(ll2xy(rect.bottomRight()).toPoint()));
			if (sbr.size().width() >= size.width() || sbr.size().height()
			  >= size.height()) {
				_zoom--;
				break;
			}
		}
	}

	return _zoom;
}

int JNXMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	return _zoom;
}

int JNXMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	return _zoom;
}

QPixmap JNXMap::pixmap(const Tile *tile, QFile *file)
{
	QPixmap pm;

	QString key = file->fileName() + "-" + QString::number(tile->offset);
	if (!QPixmapCache::find(key, &pm)) {
		QByteArray ba;
		ba.resize(tile->size + 2);
		ba[0] = (char)0xFF;
		ba[1] = (char)0xD8;
		char *data = ba.data() + 2;

		if (!file->seek(tile->offset))
			return QPixmap();
		if (!file->read(data, tile->size))
			return QPixmap();
		pm = QPixmap::fromImage(QImage::fromData(ba));

		if (!pm.isNull())
			QPixmapCache::insert(key, pm);
	}

	return pm;
}

bool JNXMap::cb(Tile *tile, void *context)
{
	Ctx *ctx = static_cast<Ctx*>(context);
	QPixmap pm(pixmap(tile, ctx->file));
#ifdef ENABLE_HIDPI
	pm.setDevicePixelRatio(ctx->ratio);
#endif // ENABLE_HIDPI
	ctx->painter->drawPixmap(tile->pos / ctx->ratio, pm);

	return true;
}

void JNXMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	const RTree<Tile*, qreal, 2> &tree = _zooms.at(_zoom).tree;
	Ctx ctx(painter, &_file, _ratio);
	QRectF rr(rect.topLeft() * _ratio, rect.size() * _ratio);

	qreal min[2], max[2];
	min[0] = rr.left();
	min[1] = rr.top();
	max[0] = rr.right();
	max[1] = rr.bottom();
	tree.Search(min, max, cb, &ctx);
}
