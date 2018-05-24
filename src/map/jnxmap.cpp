#include <QtEndian>
#include <QPainter>
#include <QFileInfo>
#include "transform.h"
#include "rectd.h"
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

	Ctx(QPainter *painter, QFile *file) : painter(painter), file(file) {}
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

			RectD rect(PointD(ic2dc(left), ic2dc(top)), PointD(ic2dc(right),
			  ic2dc(bottom)));

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
  : Map(parent), _file(fileName), _zoom(0), _valid(false)
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

QPointF JNXMap::ll2xy(const Coordinates &c) const
{
	const Transform &t = _zooms.at(_zoom).transform;
	return t.proj2img(PointD(c.lon(), c.lat()));
}

Coordinates JNXMap::xy2ll(const QPointF &p) const
{
	const Transform &t = _zooms.at(_zoom).transform;
	PointD pp(t.img2proj(p));
	return Coordinates(pp.x(), pp.y());
}

QRectF JNXMap::bounds() const
{
	const Transform &t = _zooms.at(_zoom).transform;

	return QRectF(t.proj2img(PointD(_bounds.topLeft().lon(),
	  _bounds.topLeft().lat())), t.proj2img(PointD(_bounds.bottomRight().lon(),
	  _bounds.bottomRight().lat())));
}

qreal JNXMap::resolution(const QRectF &rect) const
{
	Coordinates tl = xy2ll((rect.topLeft()));
	Coordinates br = xy2ll(rect.bottomRight());

	qreal ds = tl.distanceTo(br);
	qreal ps = QLineF(rect.topLeft(), rect.bottomRight()).length();

	return ds/ps;
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
	QByteArray ba;
	ba.resize(tile->size + 2);
	ba[0] = (char)0xFF;
	ba[1] = (char)0xD8;
	char *data = ba.data() + 2;

	if (!file->seek(tile->offset))
		return QPixmap();
	if (!file->read(data, tile->size))
		return QPixmap();

	return QPixmap::fromImage(QImage::fromData(ba));
}

bool JNXMap::cb(Tile *tile, void *context)
{
	Ctx *ctx = static_cast<Ctx*>(context);
	ctx->painter->drawPixmap(tile->pos, pixmap(tile, ctx->file));

	return true;
}

void JNXMap::draw(QPainter *painter, const QRectF &rect, bool block)
{
	Q_UNUSED(block);
	const RTree<Tile*, qreal, 2> &tree = _zooms.at(_zoom).tree;
	Ctx ctx(painter, &_file);

	qreal min[2], max[2];
	min[0] = rect.left();
	min[1] = rect.top();
	max[0] = rect.right();
	max[1] = rect.bottom();
	tree.Search(min, max, cb, &ctx);
}
