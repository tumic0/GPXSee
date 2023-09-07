#include <QPainter>
#include <QPixmapCache>
#include "common/range.h"
#include "common/wgs84.h"
#include "rectd.h"
#include "pcs.h"
#include "encjob.h"
#include "encmap.h"


using namespace ENC;

#define TILE_SIZE 512

static Range zooms(const RectC &bounds)
{
	double size = qMin(bounds.width(), bounds.height());

	if (size > 180)
		return Range(0, 10);
	else if (size > 90)
		return Range(1, 11);
	else if (size > 45)
		return Range(2, 12);
	else if (size > 22.5)
		return Range(3, 13);
	else if (size > 11.25)
		return Range(4, 14);
	else if (size > 5.625)
		return Range(5, 15);
	else if (size > 2.813)
		return Range(6, 16);
	else if (size > 1.406)
		return Range(7, 17);
	else if (size > 0.703)
		return Range(8, 18);
	else if (size > 0.352)
		return Range(9, 19);
	else if (size > 0.176)
		return Range(10, 20);
	else if (size > 0.088)
		return Range(11, 20);
	else if (size > 0.044)
		return Range(12, 20);
	else if (size > 0.022)
		return Range(13, 20);
	else if (size > 0.011)
		return Range(14, 20);
	else
		return Range(15, 20);
}

static const ISO8211::Field *SGXD(const ISO8211::Record &r)
{
	const ISO8211::Field *f;

	if ((f = ISO8211::field(r, "SG2D")))
		return f;
	else if ((f = ISO8211::field(r, "SG3D")))
		return f;
	else
		return 0;
}

bool ENCMap::bounds(const ISO8211::Record &record, Rect &rect)
{
	bool xok, yok;
	// edge geometries can be empty!
	const ISO8211::Field *f = SGXD(record);
	if (!f)
		return true;

	for (int i = 0; i < f->data().size(); i++) {
		const QVector<QVariant> &c = f->data().at(i);
		rect.unite(c.at(1).toInt(&xok), c.at(0).toInt(&yok));
		if (!(xok && yok))
			return false;
	}

	return true;
}

bool ENCMap::bounds(const QVector<ISO8211::Record> &gv, Rect &b)
{
	Rect r;

	for (int i = 0; i < gv.size(); i++) {
		if (!bounds(gv.at(i), r))
			return false;
		b |= r;
	}

	return true;
}

bool ENCMap::processRecord(const ISO8211::Record &record,
  QVector<ISO8211::Record> &rv, uint &COMF, QString &name)
{
	if (record.size() < 2)
		return false;

	const ISO8211::Field &f = record.at(1);
	const QByteArray &ba = f.tag();

	if (ba == "VRID") {
		rv.append(record);
	} else if (ba == "DSID") {
		QByteArray DSNM;
		if (!f.subfield("DSNM", &DSNM))
			return false;
		name = DSNM;
	} else if (ba == "DSPM") {
		if (!f.subfield("COMF", &COMF))
			return false;
	}

	return true;
}

ENCMap::ENCMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _data(0), _projection(PCS::pcs(3857)),
  _tileRatio(1.0), _valid(false)
{
	QVector<ISO8211::Record> gv;
	ISO8211 ddf(fileName);
	ISO8211::Record record;
	uint COMF = 1;

	if (!ddf.readDDR()) {
		_errorString = ddf.errorString();
		return;
	}
	while (ddf.readRecord(record)) {
		if (!processRecord(record, gv, COMF, _name)) {
			_errorString = "Invalid S-57 record";
			return;
		}
	}
	if (!ddf.errorString().isNull()) {
		_errorString = ddf.errorString();
		return;
	}

	Rect b;
	if (!bounds(gv, b)) {
		_errorString = "Error fetching geometries bounds";
		return;
	}
	Coordinates tl(b.minX() / (double)COMF, b.maxY() / (double)COMF);
	Coordinates br(b.maxX() / (double)COMF, b.minY() / (double)COMF);
	_llBounds = RectC(tl, br);
	if (!_llBounds.isValid()) {
		_errorString = "Invalid geometries bounds";
		return;
	}

	_zooms = zooms(_llBounds);
	_zoom = _zooms.min();
	updateTransform();

	_valid = true;
}

void ENCMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi)
{
	Q_UNUSED(in);
	Q_UNUSED(hidpi);

	_tileRatio = deviceRatio;
	_projection = out;
	Q_ASSERT(!_data);
	_data = new MapData(path());
	QPixmapCache::clear();
}

void ENCMap::unload()
{
	cancelJobs(true);

	delete _data;
	_data = 0;
}

int ENCMap::zoomFit(const QSize &size, const RectC &rect)
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

int ENCMap::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _zooms.max());
	updateTransform();
	return _zoom;
}

int ENCMap::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, _zooms.min());
	updateTransform();
	return _zoom;
}

void ENCMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform ENCMap::transform(int zoom) const
{
	int z = zoom + Util::log2i(TILE_SIZE);

	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<z) : (2.0 * M_PI * WGS84_RADIUS) / (1<<z);
	PointD topLeft(_projection.ll2xy(_llBounds.topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void ENCMap::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_llBounds, _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
}

bool ENCMap::isRunning(int zoom, const QPoint &xy) const
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

void ENCMap::runJob(ENCJob *job)
{
	_jobs.append(job);

	connect(job, &ENCJob::finished, this, &ENCMap::jobFinished);
	job->run();
}

void ENCMap::removeJob(ENCJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void ENCMap::jobFinished(ENCJob *job)
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

void ENCMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

QString ENCMap::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

void ENCMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
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
				tiles.append(RasterTile(_projection, _transform, _data,
				  _zoom, _zooms, QRect(ttl, QSize(TILE_SIZE, TILE_SIZE)),
				  _tileRatio));
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

Map *ENCMap::create(const QString &path, bool *isMap)
{
	if (isMap)
		*isMap = false;

	return new ENCMap(path);
}
