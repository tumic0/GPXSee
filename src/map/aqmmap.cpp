#include <cctype>
#include <QPainter>
#include <QPixmapCache>
#include <QImageReader>
#include <QBuffer>
#include <QtConcurrent>
#include "osm.h"
#include "aqmmap.h"


#define MAGIC "FLATPACK1"

class AQTile
{
public:
	AQTile(const QPoint &xy, const QByteArray &data, const QString &key)
	  : _xy(xy), _data(data), _key(key) {}

	const QPoint &xy() const {return _xy;}
	const QString &key() const {return _key;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {_pixmap.loadFromData(_data);}

private:
	QPoint _xy;
	QByteArray _data;
	QString _key;
	QPixmap _pixmap;
};


static bool parseHeader(const QByteArray &data, QString &name)
{
	QList<QByteArray> lines = data.split('\n');

	for (int i = 0; i < lines.count(); i++) {
		const QByteArray &line = lines.at(i);

		QList<QByteArray> tokens = line.split('=');
		if (tokens.size() != 2)
			continue;

		QByteArray key(tokens.at(0).trimmed());
		QByteArray value(tokens.at(1).trimmed());

		if (key == "name")
			name = value;
	}

	return !name.isEmpty();
}

static bool parseLevel(const QByteArray &data, int &zoom, int &tileSize,
  QRect &rect)
{
	int id = -1;
	int xtsize = 0, ytsize = 0;
	int xtmin = 0, xtmax = 0, ytmin = 0, ytmax = 0;
	QList<QByteArray> lines = data.split('\n');

	for (int i = 0; i < lines.count(); i++) {
		const QByteArray &line = lines.at(i);

		QList<QByteArray> tokens = line.split('=');
		if (tokens.size() != 2)
			continue;

		QByteArray key(tokens.at(0).trimmed());
		QByteArray value(tokens.at(1).trimmed());
		bool ok = true;

		if (key == "id")
			id = value.toInt(&ok);
		else if (key == "xtsize")
			xtsize = value.toInt(&ok);
		else if (key == "ytsize")
			ytsize = value.toInt(&ok);
		else if (key == "xtmin")
			xtmin = value.toInt(&ok);
		else if (key == "xtmax")
			xtmax = value.toInt(&ok);
		else if (key == "ytmin")
			ytmin = value.toInt(&ok);
		else if (key == "ytmax")
			ytmax = value.toInt(&ok);

		if (!ok)
			return false;
	}

	if (xtsize <= 0 || ytsize <= 0 || ytsize != xtsize || id < 0)
		return false;

	zoom = id;
	tileSize = xtsize;
	rect = QRect(QPoint(xtmin, (1<<zoom) - ytmax /*- 1*/),
	  QPoint(xtmax, (1<<zoom) - ytmin /*- 1*/));

	return true;
}


bool AQMMap::readSize(size_t &size)
{
	char c;

	size = 0;

	while (_file.getChar(&c)) {
		if (isdigit(c))
			size = size * 10 + c - '0';
		else if (!c)
			return true;
		else
			return false;
	}

	return false;
}

bool AQMMap::readString(QByteArray &str)
{
	char c;

	str.clear();

	while (_file.getChar(&c)) {
		if (!c)
			return true;
		else
			str.append(c);
	}

	return false;
}

bool AQMMap::readFile(File &file)
{
	if (!readString(file.name))
		return false;
	if (!readSize(file.offset))
		return false;

	return true;
}

bool AQMMap::readData(QByteArray &data)
{
	size_t size;

	if (!readSize(size))
		return false;
	data.resize(size);
	return _file.read(data.data(), size) == (qint64)size;
}

bool AQMMap::readHeader()
{
	size_t hdrSize, numFiles;
	QByteArray data;

	if (!readSize(hdrSize))
		return false;
	if (!readSize(numFiles))
		return false;

	QVector<File> files(numFiles);
	for (size_t i = 0; i < numFiles; i++) {
		if (!readFile(files[i]))
			return false;
	}

	size_t start = _file.pos();
	for (int i = 0; i < files.size(); i++)
		files[i].offset += start;

	int li = -1;
	for (int i = 0; i < files.size(); i++) {
		if (files.at(i).name == "V2HEADER") {
			if (!(_file.seek(files.at(i).offset) && readData(data)))
				return false;
			if (!parseHeader(data, _name))
				return false;
		} else if (files.at(i).name == "V2LEVEL") {
			int zoom, tileSize;
			QRect bounds;

			if (!(_file.seek(files.at(i).offset) && readData(data)))
				return false;
			if (!parseLevel(data, zoom, tileSize, bounds))
				return false;

			if (_bounds.isNull()) {
				int minX = qMin((1<<zoom) - 1, qMax(0, bounds.left()));
				int minY = qMin((1<<zoom) - 1, qMax(0, bounds.top()));
				int maxX = qMin((1<<zoom) - 1, qMax(0, bounds.right())) + 1;
				int maxY = qMin((1<<zoom) - 1, qMax(0, bounds.bottom())) + 1;
				Coordinates tl(OSM::tile2ll(QPoint(minX, minY), zoom));
				tl.rlat() = -tl.lat();
				Coordinates br(OSM::tile2ll(QPoint(maxX, maxY), zoom));
				br.rlat() = -br.lat();
				// Workaround of broken zoom levels 0 and 1 due to numerical
				// instability
				tl.rlat() = qMin(tl.lat(), OSM::BOUNDS.top());
				br.rlat() = qMax(br.lat(), OSM::BOUNDS.bottom());
				_bounds = RectC(tl, br);
			}
			_zooms.append(Zoom(zoom, tileSize));
		} else if (files.at(i).name == "@LEVEL") {
			li = i;
			break;
		}
	}

	if (li < 0)
		return false;

	int level = -1;
	for (int i = li; i < files.size(); i++) {
		if (files.at(i).name == "@LEVEL")
			level++;
		else if (files.at(i).name == "#END")
			break;
		else {
			if (level < 0 || level > _zooms.size() - 1)
				return false;

			QList<QByteArray> ba(files.at(i).name.split('_'));
			if (ba.size() != 2)
				return false;
			bool xok, yok;
			int x = ba.at(0).toInt(&xok);
			int y = ba.at(1).toInt(&yok);
			if (!(xok && yok))
				return false;
			int zoom = _zooms.at(level).zoom;
			_zooms[level].tiles.insert(QPoint(x, (1<<zoom) - y /*- 1*/),
			  files.at(i).offset);
		}
	}

	return true;
}

AQMMap::AQMMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _file(fileName), _zoom(0), _mapRatio(1.0),
  _valid(false)
{
	char magic[sizeof(MAGIC) - 1];

	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	if (_file.read(magic, sizeof(magic)) != sizeof(magic)
	  || memcmp(magic, MAGIC, sizeof(magic))) {
		_errorString = "Not an AlpineQuest map";
		return;
	}

	if (!readHeader()) {
		_errorString = "AQM file format error";
		return;
	}

	_file.close();

	_valid = true;
}

void AQMMap::load()
{
	_file.open(QIODevice::ReadOnly);
}

void AQMMap::unload()
{
	_file.close();
}

QRectF AQMMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int AQMMap::zoomFit(const QSize &size, const RectC &rect)
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

qreal AQMMap::resolution(const QRectF &rect)
{
	const Zoom &z = _zooms.at(_zoom);
	return OSM::resolution(rect.center(), z.zoom, z.tileSize);
}

int AQMMap::zoomIn()
{
	_zoom = qMin(_zoom + 1, _zooms.size() - 1);
	return _zoom;
}

int AQMMap::zoomOut()
{
	_zoom = qMax(_zoom - 1, 0);
	return _zoom;
}

void AQMMap::setDevicePixelRatio(qreal deviceRatio, qreal mapRatio)
{
	Q_UNUSED(deviceRatio);
	_mapRatio = mapRatio;
}

QPointF AQMMap::ll2xy(const Coordinates &c)
{
	const Zoom &z = _zooms.at(_zoom);
	qreal scale = OSM::zoom2scale(z.zoom, z.tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / _mapRatio;
}

Coordinates AQMMap::xy2ll(const QPointF &p)
{
	const Zoom &z = _zooms.at(_zoom);
	qreal scale = OSM::zoom2scale(z.zoom, z.tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale) * _mapRatio);
}

qreal AQMMap::tileSize() const
{
	return (_zooms.at(_zoom).tileSize / _mapRatio);
}

QByteArray AQMMap::tileData(const QPoint &tile)
{
	const Zoom &z = _zooms.at(_zoom);
	QByteArray ba;

	size_t offset = z.tiles.value(tile);
	if (!(offset && _file.seek(offset) && readData(ba)))
		return QByteArray();

	return ba;
}

void AQMMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	const Zoom &z = _zooms.at(_zoom);
	qreal scale = OSM::zoom2scale(z.zoom, z.tileSize);
	QRectF b(bounds());


	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * _mapRatio, z.zoom);
	QPointF tl(floor(rect.left() / tileSize())
	  * tileSize(), floor(rect.top() / tileSize()) * tileSize());

	QSizeF s(qMin(rect.right() - tl.x(), b.width()),
	  qMin(rect.bottom() - tl.y(), b.height()));
	int width = ceil(s.width() / tileSize());
	int height = ceil(s.height() / tileSize());


	QList<AQTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(z.zoom) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(qMax(tl.x(), b.left()) + (t.x() - tile.x())
				  * tileSize(), qMax(tl.y(), b.top()) + (t.y() - tile.y())
				  * tileSize());
				drawTile(painter, pm, tp);
			} else {
				tiles.append(AQTile(t, tileData(t), key));
			}
		}
	}

	QFuture<void> future = QtConcurrent::map(tiles, &AQTile::load);
	future.waitForFinished();

	for (int i = 0; i < tiles.size(); i++) {
		const AQTile &mt = tiles.at(i);
		QPixmap pm(mt.pixmap());
		if (pm.isNull())
			continue;

		QPixmapCache::insert(mt.key(), pm);

		QPointF tp(qMax(tl.x(), b.left()) + (mt.xy().x() - tile.x())
		  * tileSize(), qMax(tl.y(), b.top()) + (mt.xy().y() - tile.y())
		  * tileSize());
		drawTile(painter, pm, tp);
	}
}

void AQMMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(_mapRatio);
	painter->drawPixmap(tp, pixmap);
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const AQMMap::File &file)
{
	dbg.nospace() << "File(" << file.name << ", " << file.offset << ")";
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const AQMMap::Zoom &zoom)
{
	dbg.nospace() << "Zoom(" << zoom.zoom << ", " << zoom.tileSize << ", "
	  << zoom.tiles << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG
