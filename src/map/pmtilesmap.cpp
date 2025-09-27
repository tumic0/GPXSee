#include <QPainter>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmapCache>
#include "osm.h"
#include "pmtilesmap.h"

#define MAX_TILE_SIZE   4096
#define LEAF_CACHE_SIZE 16

struct Header {
	quint64 magic;
	quint64 rootOffset;
	quint64 rootLength;
	quint64 metadataOffset;
	quint64 metadataLength;
	quint64 leafOffset;
	quint64 leafLength;
	quint64 tileOffset;
	quint64 tileLength;
	quint64 addressedTiles;
	quint64 tileEntries;
	quint64 tileContents;
	qint32 minLon;
	qint32 minLat;
	qint32 maxLon;
	qint32 maxLat;
	quint8 c;
	quint8 ic;
	quint8 tc;
	quint8 tt;
	quint8 minZ;
	quint8 maxZ;
};

template<typename T>
static bool varint(char **bp, const char *be, T &val)
{
	unsigned int shift = 0;
	val = 0;

	while ((*bp < be) && (shift < sizeof(T) * 8)) {
		val |= static_cast<T>((quint8)**bp & 0x7F) << shift;
		shift += 7;
		if (!((quint8)*(*bp)++ & 0x80))
			return true;
	}

	return false;
}

static quint64 id(unsigned z, const QPoint &p)
{
	unsigned x = p.x();
	unsigned y = p.y();
	quint64 acc = ((1 << (z * 2)) - 1) / 3;
	int a = z - 1;

	while (a >= 0) {
		quint64 s = 1 << a;
		quint64 rx = s & x;
		quint64 ry = s & y;

		acc += ((3 * rx) ^ ry) << a;
		if (ry == 0) {
			if (rx != 0) {
				x = s - 1 - x;
				y = s - 1 - y;
			}
			std::swap(x, y);
		}
		a--;
	}

	return acc;
}

QByteArray PMTilesMap::readData(quint64 offset, quint64 size, quint8 compression)
{
	QByteArray ba;

	if (!_file.seek(offset))
		return QByteArray();

	ba.resize(size);
	if (_file.read(ba.data(), ba.size()) != ba.size())
		return QByteArray();

	return (compression == 2) ? Util::gunzip(ba) : ba;
}

QVector<PMTilesMap::Directory> PMTilesMap::readDir(quint64 offset, quint64 size,
  quint8 compression)
{
	QByteArray uba(readData(offset, size, compression));
	if (uba.isNull())
		return QVector<Directory>();

	char *bp = uba.data();
	const char *be = uba.constData() + uba.size();
	quint64 n;
	if (!varint(&bp, be, n))
		return QVector<Directory>();

	QVector<Directory> dirs;
	dirs.resize(n);
	for (int i = 0; i < dirs.size(); i++) {
		quint64 tileId;
		if (!varint(&bp, be, tileId))
			return QVector<Directory>();
		dirs[i].tileId = i ? tileId + dirs[i-1].tileId : tileId;
	}
	for (int i = 0; i < dirs.size(); i++)
		if (!varint(&bp, be, dirs[i].runLength))
			return QVector<Directory>();
	for (int i = 0; i < dirs.size(); i++)
		if (!varint(&bp, be, dirs[i].length))
			return QVector<Directory>();
	for (int i = 0; i < dirs.size(); i++) {
		quint64 offset;
		if (!varint(&bp, be, offset))
			return QVector<Directory>();
		dirs[i].offset = offset
		  ? offset - 1 : dirs[i-1].offset + dirs[i-1].length;
	}

	return dirs;
}

PMTilesMap::PMTilesMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _file(fileName), _mapRatio(1.0), _tileRatio(1.0),
  _scalable(false), _scaledSize(0), _valid(false)
{
	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return;
	}

	_cache.setMaxCost(LEAF_CACHE_SIZE);

	// header
	Header hdr;
	QDataStream stream(&_file);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream >> hdr.magic >> hdr.rootOffset >> hdr.rootLength
	  >> hdr.metadataOffset >> hdr.metadataLength >> hdr.leafOffset
	  >> hdr.leafLength >> hdr.tileOffset >> hdr.tileLength
	  >> hdr.addressedTiles >> hdr.tileEntries >> hdr.tileContents
	  >> hdr.c >> hdr.ic >> hdr.tc >> hdr.tt >> hdr.minZ >> hdr.maxZ
	  >> hdr.minLon >> hdr.minLat >> hdr.maxLon >> hdr.maxLat;

	if (stream.status() || (hdr.magic & 0xFFFFFFFFFFFFFF) != 0x73656c69544d50) {
		_errorString = "Not a PMTiles file";
		return;
	}
	if ((hdr.magic >> 56) != 3) {
		_errorString = QString("%1: unsupported PMTiles version")
		  .arg(hdr.magic >> 56);
		return;
	}
	if (hdr.ic < 1 || hdr.ic > 2) {
		_errorString = QString("%1: unsupported internal compression")
		  .arg(hdr.ic);
		return;
	}
	if (hdr.tc < 1 || hdr.tc > 2) {
		_errorString = QString("%1: unsupported tile compression").arg(hdr.ic);
		return;
	}

	_bounds = RectC(Coordinates(hdr.minLon / 10000000.0, hdr.maxLat / 10000000.0),
	  Coordinates(hdr.maxLon / 10000000.0, hdr.minLat / 10000000.0));
	if (!_bounds.isValid()) {
		_errorString = "Invalid map bounds";
		return;
	}

	for (int i = hdr.minZ; i <= hdr.maxZ; i++)
		_zoomsBase.append(Zoom(i, i));
	if (_zoomsBase.isEmpty()) {
		_errorString = "Invalid zoom levels range";
		return;
	}
	_zi = _zoomsBase.size() - 1;

	_tileOffset = hdr.tileOffset;
	_leafOffset = hdr.leafOffset;
	_tc = hdr.tc;
	_ic = hdr.ic;
	_scalable = (hdr.tt == 1);

	// metadata
	if (hdr.metadataLength) {
		QByteArray uba(readData(hdr.metadataOffset, hdr.metadataLength, hdr.ic));
		if (uba.isNull())
			qWarning("%s: error reading metadata", qUtf8Printable(fileName));
		else {
			QJsonParseError error;
			QJsonDocument doc(QJsonDocument::fromJson(uba, &error));
			if (doc.isNull())
				qWarning("%s: metadata error: %s", qUtf8Printable(fileName),
				  qUtf8Printable(error.errorString()));
			else {
				QJsonObject json(doc.object());
				_name = json["name"].toString();
			}
		}
	}

	// root directory
	_root = readDir(hdr.rootOffset, hdr.rootLength, hdr.ic);
	if (_root.isEmpty()) {
		_errorString = "Error reading root directory";
		return;
	}

	// tile size
	QSize tileSize;
	if (_tc == 2) {
		QByteArray gzip(tileData(_root.first().tileId));
		QByteArray data(Util::gunzip(gzip));
		QBuffer buffer(&data);
		QImageReader reader(&buffer);
		tileSize = reader.size();
	} else {
		QByteArray data(tileData(_root.first().tileId));
		QBuffer buffer(&data);
		QImageReader reader(&buffer);
		tileSize = reader.size();
	}
	if (!tileSize.isValid() || tileSize.width() != tileSize.height()) {
		_errorString = "Unsupported/invalid tile images";
		return;
	}
	_tileSize = tileSize.width();

	_file.close();

	_valid = true;
}

void PMTilesMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, int layer)
{
	Q_UNUSED(in);
	Q_UNUSED(out);
	Q_UNUSED(layer);

	_mapRatio = hidpi ? deviceRatio : 1.0;
	_zooms = _zoomsBase;

	if (_scalable) {
		_scaledSize = _tileSize * deviceRatio;
		_tileRatio = deviceRatio;

		for (int i = _zooms.last().base + 1; i <= OSM::ZOOMS.max(); i++) {
			Zoom z(i, _zooms.last().base);
			if (_tileSize * _tileRatio * (1U<<(z.z - z.base)) > MAX_TILE_SIZE)
				break;
			_zooms.append(Zoom(i, _zooms.last().base));
		}
	}

	if (!_file.open(QIODevice::ReadOnly))
		qWarning("%s: %s", qUtf8Printable(_file.fileName()),
		  qUtf8Printable(_file.errorString()));
}

void PMTilesMap::unload()
{
	cancelJobs(true);
	_file.close();
	_cache.clear();
}

QString PMTilesMap::name() const
{
	return _name.isEmpty() ? Map::name() : _name;
}

QRectF PMTilesMap::bounds()
{
	return QRectF(ll2xy(_bounds.topLeft()), ll2xy(_bounds.bottomRight()));
}

int PMTilesMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (!rect.isValid())
		_zi = _zooms.size() - 1;
	else {
		QRectF tbr(OSM::ll2m(rect.topLeft()), OSM::ll2m(rect.bottomRight()));
		QPointF sc(tbr.width() / size.width(), tbr.height() / size.height());
		int zoom = OSM::scale2zoom(qMax(sc.x(), -sc.y()) / coordinatesRatio(),
		  _tileSize);

		_zi = 0;
		for (int i = 1; i < _zooms.size(); i++) {
			if (_zooms.at(i).z > zoom)
				break;
			_zi = i;
		}
	}

	return _zi;
}

qreal PMTilesMap::resolution(const QRectF &rect)
{
	return OSM::resolution(rect.center(), _zooms.at(_zi).z, tileSize());
}

int PMTilesMap::zoomIn()
{
	cancelJobs(false);

	_zi = qMin(_zi + 1, _zooms.size() - 1);
	return _zi;
}

int PMTilesMap::zoomOut()
{
	cancelJobs(false);

	_zi = qMax(_zi - 1, 0);
	return _zi;
}

qreal PMTilesMap::coordinatesRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio / _tileRatio : 1.0;
}

qreal PMTilesMap::imageRatio() const
{
	return _mapRatio > 1.0 ? _mapRatio : _tileRatio;
}

qreal PMTilesMap::tileSize() const
{
	return (_tileSize / coordinatesRatio());
}

const PMTilesMap::Directory *PMTilesMap::findDir(const QVector<Directory> &list,
  quint64 tileId)
{
	qint64 m = 0;
	qint64 n = list.size() - 1;

	while (m <= n) {
		quint64 k = (n + m) >> 1;
		qint64 cmp = (qint64)tileId - (qint64)list.at(k).tileId;
		if (cmp > 0)
			m = k + 1;
		else if (cmp < 0)
			n = k - 1;
		else
			return &list.at(k);
	}

	if (n >= 0) {
		if (!list.at(n).runLength)
			return &list.at(n);
		if (tileId - list.at(n).tileId < (quint64)list.at(n).runLength)
			return &list.at(n);
	}

	return 0;
}

QByteArray PMTilesMap::tileData(quint64 id)
{
	const Directory *d = findDir(_root, id);
	if (!d)
		return QByteArray();
	if (!d->runLength) {
		QVector<Directory> *leaf = _cache.object(d->offset);
		if (!leaf) {
			leaf = new QVector<Directory>(readDir(_leafOffset + d->offset,
			  d->length, _ic));
			_cache.insert(d->offset, leaf);
		}
		const Directory *l = findDir(*leaf, id);
		return (l)
		  ? readData(_tileOffset + l->offset, l->length, 1) : QByteArray();
	} else
		return readData(_tileOffset + d->offset, d->length, 1);
}

bool PMTilesMap::isRunning(const QString &key) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<PMTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++)
			if (tiles.at(j).key() == key)
				return true;
	}

	return false;
}

void PMTilesMap::runJob(PMTilesMapJob *job)
{
	_jobs.append(job);

	connect(job, &PMTilesMapJob::finished, this, &PMTilesMap::jobFinished);
	job->run();
}

void PMTilesMap::removeJob(PMTilesMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void PMTilesMap::jobFinished(PMTilesMapJob *job)
{
	const QList<PMTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const PMTile &mt = tiles.at(i);
		if (!mt.pixmap().isNull())
			QPixmapCache::insert(mt.key(), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void PMTilesMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QPointF PMTilesMap::tilePos(const QPointF &tl, const QPoint &tc,
  const QPoint &tile, unsigned overzoom) const
{
	return QPointF(tl.x() + ((tc.x() - tile.x()) << overzoom) * tileSize(),
	  tl.y() + ((tc.y() - tile.y()) << overzoom) * tileSize());
}

void PMTilesMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	const Zoom &zoom = _zooms.at(_zi);
	unsigned overzoom = zoom.z - zoom.base;
	qreal scale = OSM::zoom2scale(zoom.base, _tileSize << overzoom);
	QPoint tile = OSM::mercator2tile(QPointF(rect.topLeft().x() * scale,
	  -rect.topLeft().y() * scale) * coordinatesRatio(), zoom.base);
	QPointF tlm(OSM::tile2mercator(tile, zoom.base));
	QPointF tl(QPointF(tlm.x() / scale, tlm.y() / scale) / coordinatesRatio());
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	unsigned f = 1U<<overzoom;
	int width = ceil(s.width() / (tileSize() * f));
	int height = ceil(s.height() / (tileSize() * f));

	QList<PMTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint t(tile.x() + i, tile.y() + j);
			QString key = path() + "-" + QString::number(zoom.z) + "_"
			  + QString::number(t.x()) + "_" + QString::number(t.y());

			if (isRunning(key))
				continue;

			if (QPixmapCache::find(key, &pm)) {
				QPointF tp(tilePos(tl, t, tile, overzoom));
				drawTile(painter, pm, tp);
			} else
				tiles.append(PMTile(zoom.z, overzoom, _scaledSize, t,
				  tileData(id(zoom.base, t)), _tc, key));
		}
	}

	if (!tiles.isEmpty()) {
		if (flags & Map::Block || !_scalable) {
			QFuture<void> future = QtConcurrent::map(tiles, &PMTile::load);
			future.waitForFinished();

			for (int i = 0; i < tiles.size(); i++) {
				const PMTile &mt = tiles.at(i);
				QPixmap pm(mt.pixmap());
				if (pm.isNull())
					continue;

				QPixmapCache::insert(mt.key(), pm);

				QPointF tp(tilePos(tl, mt.xy(), tile, overzoom));
				drawTile(painter, pm, tp);
			}
		} else
			runJob(new PMTilesMapJob(tiles));
	}
}

void PMTilesMap::drawTile(QPainter *painter, QPixmap &pixmap, QPointF &tp)
{
	pixmap.setDevicePixelRatio(imageRatio());
	painter->drawPixmap(tp, pixmap);
}

QPointF PMTilesMap::ll2xy(const Coordinates &c)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zi).z, _tileSize);
	QPointF m = OSM::ll2m(c);
	return QPointF(m.x() / scale, m.y() / -scale) / coordinatesRatio();
}

Coordinates PMTilesMap::xy2ll(const QPointF &p)
{
	qreal scale = OSM::zoom2scale(_zooms.at(_zi).z, _tileSize);
	return OSM::m2ll(QPointF(p.x() * scale, -p.y() * scale)
	  * coordinatesRatio());
}

Map *PMTilesMap::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = false;

	return new PMTilesMap(path);
}
