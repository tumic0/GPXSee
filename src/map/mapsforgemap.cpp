#include <QPainter>
#include <QPixmapCache>
#include <QDir>
#include "common/wgs84.h"
#include "common/util.h"
#include "common/programpaths.h"
#include "rectd.h"
#include "pcs.h"
#include "mapsforgemap.h"


using namespace Mapsforge;

#define EPSILON     1e-6

MapsforgeMap::MapsforgeMap(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _data(fileName), _style(0), _zoom(0),
  _projection(PCS::pcs(3857)), _tileRatio(1.0)
{
	if (_data.isValid())
		_zoom = _data.zooms().min();
}

MapsforgeMap::~MapsforgeMap()
{
	delete _style;
}

void MapsforgeMap::load(const Projection &in, const Projection &out,
  qreal deviceRatio, bool hidpi, int style, int layer)
{
	Q_UNUSED(in);
	Q_UNUSED(hidpi);

	_tileRatio = deviceRatio;
	_projection = out;

	_data.load();

	if (style < 0 || style >= styles().size())
		style = 0;
	_style = new Style(styles().at(style), _data, _tileRatio, layer);

	updateTransform();

	QPixmapCache::clear();
}

void MapsforgeMap::unload()
{
	cancelJobs(true);

	_data.clear();
	delete _style;
	_style = 0;
}

int MapsforgeMap::zoomFit(const QSize &size, const RectC &rect)
{
	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		_zoom = _data.zooms().min();
		for (int i = _data.zooms().min() + 1; i <= _data.zooms().max(); i++) {
			Transform t(transform(i));
			QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));
			if (size.width() + EPSILON < r.width()
			  || size.height() + EPSILON < r.height())
				break;
			_zoom = i;
		}
	} else
		_zoom = _data.zooms().max();

	updateTransform();

	return _zoom;
}

int MapsforgeMap::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _data.zooms().max());
	updateTransform();
	return _zoom;
}

int MapsforgeMap::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, _data.zooms().min());
	updateTransform();
	return _zoom;
}

void MapsforgeMap::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform MapsforgeMap::transform(int zoom) const
{
	int z = zoom + Util::log2i(_data.tileSize());

	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<z) : (2.0 * M_PI * WGS84_RADIUS) / (1<<z);
	PointD topLeft(_projection.ll2xy(_data.bounds().topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void MapsforgeMap::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_data.bounds(), _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
	// Adjust the bounds of world maps to avoid problems with wrapping
	if (_data.bounds().left() <= -180.0 || _data.bounds().right() >= 180.0)
		_bounds.adjust(0.5, 0, -0.5, 0);
}

QString MapsforgeMap::key(int zoom, const QPoint &xy) const
{
	return path() + "-" + QString::number(zoom) + "_"
	  + QString::number(xy.x()) + "_" + QString::number(xy.y());
}

bool MapsforgeMap::isRunning(int zoom, const QPoint &xy) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<Mapsforge::RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++) {
			const Mapsforge::RasterTile &mt = tiles.at(j);
			if (mt.zoom() == zoom && mt.xy() == xy)
				return true;
		}
	}

	return false;
}

void MapsforgeMap::runJob(MapsforgeMapJob *job)
{
	_jobs.append(job);

	connect(job, &MapsforgeMapJob::finished, this, &MapsforgeMap::jobFinished);
	job->run();
}

void MapsforgeMap::removeJob(MapsforgeMapJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void MapsforgeMap::jobFinished(MapsforgeMapJob *job)
{
	const QList<Mapsforge::RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const Mapsforge::RasterTile &mt = tiles.at(i);
		if (!mt.pixmap().isNull())
			QPixmapCache::insert(key(mt.zoom(), mt.xy()), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void MapsforgeMap::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

void MapsforgeMap::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	int tileSize = _data.tileSize();
	QPointF tl(floor(rect.left() / tileSize) * tileSize,
	  floor(rect.top() / tileSize) * tileSize);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / tileSize);
	int height = ceil(s.height() / tileSize);

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPoint ttl(tl.x() + i * tileSize, tl.y() + j * tileSize);
			if (isRunning(_zoom, ttl))
				continue;

			QPixmap pm;
			if (QPixmapCache::find(key(_zoom, ttl), &pm))
				painter->drawPixmap(ttl, pm);
			else {
				tiles.append(RasterTile(_projection, _transform, _style, &_data,
				  _zoom, QRect(ttl, QSize(tileSize, tileSize)), _tileRatio,
				  flags & Map::HillShading));
			}
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
			runJob(new MapsforgeMapJob(tiles));
	}
}

QStringList MapsforgeMap::styles(int &defaultStyle) const
{
	QStringList list;
	list.reserve(styles().size());

	for (int i = 0; i < styles().size(); i++)
		list.append(QFileInfo(styles().at(i)).baseName());

	defaultStyle = 0;

	return list;
}

QStringList MapsforgeMap::layers(const QString &lang, int &defaultLayer) const
{
	return _style->layers(lang, defaultLayer);
}

bool MapsforgeMap::hillShading() const
{
	return _style->hasHillShading();
}

Map *MapsforgeMap::create(const QString &path, const Projection &proj,
  bool *isMap)
{
	Q_UNUSED(proj);

	if (isMap)
		*isMap = false;

	return new MapsforgeMap(path);
}

MapsforgeMap::StyleList::StyleList()
{
	QString path(ProgramPaths::styleDir());
	if (!path.isEmpty()) {
		QDir dir(path);
		QFileInfoList styles(dir.entryInfoList(QDir::Dirs
		  | QDir::NoDotAndDotDot));

		for (int i = 0; i < styles.size(); i++) {
			QDir d(styles.at(i).absoluteFilePath());
			QFileInfoList files(d.entryInfoList(QStringList("*.xml"),
			  QDir::Files));

			for (int j = 0; j < files.size(); j++)
				append(files.at(j).absoluteFilePath());
		}
	}

	if (isEmpty())
		append(":/style/GPXSee.xml");
}

MapsforgeMap::StyleList &MapsforgeMap::styles()
{
	static StyleList list;
	return list;
}
