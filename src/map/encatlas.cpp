#include <QPainter>
#include <QPixmapCache>
#include "common/wgs84.h"
#include "GUI/format.h"
#include "rectd.h"
#include "pcs.h"
#include "encjob.h"
#include "encatlas.h"

using namespace ENC;

#define TILE_SIZE 512

Range ENCAtlas::zooms(IntendedUsage usage)
{
	switch (usage) {
		case Overview:
			return Range(6, 7);
		case General:
			return Range(8, 9);
		case Coastal:
			return Range(10, 11);
		case Approach:
			return Range(12, 13);
		case Harbour:
			return Range(14, 18);
		case Berthing:
			return Range(19, 19);

		case River:
			return Range(12, 17);
		case RiverHarbour:
			return Range(18, 18);
		case RiverBerthing:
			return Range(19, 19);
		default:
			return Range(0, 19);
	};
}

ENCAtlas::IntendedUsage ENCAtlas::usage(const QString &path)
{
	QFileInfo fi(path);
	QString basename(fi.baseName());

	if (basename.size() != 8)
		return Unknown;
	int iu = basename.at(2).digitValue();
	if (iu < 1 || iu > 9)
		return Unknown;

	return (IntendedUsage)iu;
}

bool ENCAtlas::processRecord(const ISO8211::Record &record, QByteArray &file,
  RectC &bounds)
{
	if (record.size() < 2)
		return false;

	const ENC::ISO8211::Field &f = record.at(1);
	const QByteArray &ba = f.tag();

	if (ba == "CATD") {
		QByteArray FILE, IMPL;

		if (!f.subfield("IMPL", &IMPL))
			return false;
		if (!f.subfield("FILE", &FILE))
			return false;

		if (IMPL == "BIN" && FILE.endsWith("000")) {
			QByteArray SLAT, WLON, NLAT, ELON;

			if (!f.subfield("SLAT", &SLAT))
				return false;
			if (!f.subfield("WLON", &WLON))
				return false;
			if (!f.subfield("NLAT", &NLAT))
				return false;
			if (!f.subfield("ELON", &ELON))
				return false;

			bool ok1, ok2, ok3, ok4;
			bounds = RectC(Coordinates(WLON.toDouble(&ok1), NLAT.toDouble(&ok2)),
			  Coordinates(ELON.toDouble(&ok3), SLAT.toDouble(&ok4)));
			if (!(ok1 && ok2 && ok3 && ok4))
				return false;

			file = FILE.replace('\\', '/');

			return true;
		}
	}

	return false;
}

void ENCAtlas::addMap(const QDir &dir, const QByteArray &file,
  const RectC &bounds)
{
	QString path(dir.absoluteFilePath(file));
	if (!QFileInfo::exists(path)) {
		qWarning("%s: No such map file", qPrintable(path));
		return;
	}
	if (!bounds.isValid()) {
		qWarning("%s: Invalid map bounds", qPrintable(path));
		return;
	}

	IntendedUsage iu = usage(path);
	auto it = _data.find(iu);
	if (it == _data.end())
		it = _data.insert(iu, new AtlasData(_cache, _lock));

	it.value()->addMap(bounds, path);

	_name = "ENC (" + Format::coordinates(bounds.center(), DecimalDegrees) + ")";
	_llBounds |= bounds;
}

ENCAtlas::ENCAtlas(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _projection(PCS::pcs(3857)),
  _tileRatio(1.0), _zoom(0), _valid(false)
{
	QDir dir(QFileInfo(fileName).absoluteDir());
	ISO8211 ddf(fileName);
	ISO8211::Record record;
	QByteArray file;
	RectC bounds;

	if (!ddf.readDDR()) {
		_errorString = ddf.errorString();
		return;
	}
	while (ddf.readRecord(record)) {
		if (processRecord(record, file, bounds))
			addMap(dir, file, bounds);
	}
	if (!ddf.errorString().isNull()) {
		_errorString = ddf.errorString();
		return;
	}

	if (_data.isEmpty()) {
		_errorString = "No usable ENC map found";
		return;
	}

	_usage = _data.firstKey();
	_zoom = zooms(_usage).min();
	updateTransform();

	_cache.setMaxCost(10);

	_valid = true;
}

ENCAtlas::~ENCAtlas()
{
	qDeleteAll(_data);
}

void ENCAtlas::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(hidpi);

	_tileRatio = deviceRatio;
	_projection = out;

	QPixmapCache::clear();
}

void ENCAtlas::unload()
{
	cancelJobs(true);

	_cache.clear();
}

int ENCAtlas::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		for (auto it = _data.cbegin(); it != _data.cend(); ++it) {
			Range z(zooms(it.key()));

			_usage = it.key();
			_zoom = z.min();

			for (int i = z.min() + 1; i <= z.max(); i++) {
				Transform t(transform(i));
				QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));

				if (size.width() < r.width() || size.height() < r.height()) {
					updateTransform();
					return _zoom;
				}

				_zoom = i;
			}
		}
	} else {
		IntendedUsage usage(_data.lastKey());
		_usage = usage;
		_zoom = zooms(usage).max();
	}

	updateTransform();

	return _zoom;
}

int ENCAtlas::zoomIn()
{
	cancelJobs(false);

	if (_zoom + 1 <= zooms(_usage).max())
		_zoom++;
	else {
		auto it = _data.find(_usage);
		if (++it != _data.end()) {
			_usage = it.key();
			_zoom = zooms(it.key()).min();
		}
	}

	updateTransform();

	return _zoom;
}

int ENCAtlas::zoomOut()
{
	cancelJobs(false);

	if (_zoom - 1 >= zooms(_usage).min())
		_zoom--;
	else {
		auto it = _data.find(_usage);
		if (it != _data.begin()) {
			--it;
			_usage = it.key();
			_zoom = zooms(it.key()).max();
		}
	}

	updateTransform();

	return _zoom;
}

void ENCAtlas::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform ENCAtlas::transform(int zoom) const
{
	int z = zoom + Util::log2i(TILE_SIZE);

	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<z) : (2.0 * M_PI * WGS84_RADIUS) / (1<<z);
	PointD topLeft(_projection.ll2xy(_llBounds.topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void ENCAtlas::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_llBounds, _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
}

bool ENCAtlas::isRunning(int zoom, const QPoint &xy) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<ENC::RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++) {
			const ENC::RasterTile &mt = tiles.at(j);
			if (mt.zoom() == zoom && mt.xy() == xy)
				return true;
		}
	}

	return false;
}

void ENCAtlas::runJob(ENCJob *job)
{
	_jobs.append(job);

	connect(job, &ENCJob::finished, this, &ENCAtlas::jobFinished);
	job->run();
}

void ENCAtlas::removeJob(ENCJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void ENCAtlas::jobFinished(ENCJob *job)
{
	const QList<ENC::RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const ENC::RasterTile &mt = tiles.at(i);
		if (mt.isValid())
			QPixmapCache::insert(key(mt.zoom(), mt.xy()), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void ENCAtlas::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QString ENCAtlas::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

void ENCAtlas::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	Q_UNUSED(flags);
	QPointF tl(floor(rect.left() / TILE_SIZE) * TILE_SIZE,
	  floor(rect.top() / TILE_SIZE) * TILE_SIZE);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / TILE_SIZE);
	int height = ceil(s.height() / TILE_SIZE);

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPoint ttl(tl.x() + i * TILE_SIZE, tl.y() + j * TILE_SIZE);
			if (isRunning(_zoom, ttl))
				continue;

			QPixmap pm;
			if (QPixmapCache::find(key(_zoom, ttl), &pm))
				painter->drawPixmap(ttl, pm);
			else
				tiles.append(RasterTile(_projection, _transform,
				  _data.value(_usage), _zoom, zooms(_usage),
				  QRect(ttl, QSize(TILE_SIZE, TILE_SIZE)), _tileRatio));
		}
	}

	if (!tiles.isEmpty()) {
		if (flags & Map::Block) {
			QFuture<void> future = QtConcurrent::map(tiles, &RasterTile::render);
			future.waitForFinished();

			for (int i = 0; i < tiles.size(); i++) {
				const RasterTile &mt = tiles.at(i);
				const QPixmap &pm = mt.pixmap();
				painter->drawPixmap(mt.xy(), pm);
				QPixmapCache::insert(key(mt.zoom(), mt.xy()), pm);
			}
		} else
			runJob(new ENCJob(tiles));
	}
}

Map *ENCAtlas::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = true;

	return new ENCAtlas(path);
}
