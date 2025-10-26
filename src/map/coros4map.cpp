#include <QFile>
#include <QPainter>
#include <QPixmapCache>
#include "common/wgs84.h"
#include "common/programpaths.h"
#include "IMG/imgdata.h"
#include "IMG/demtree.h"
#include "rectd.h"
#include "pcs.h"
#include "imgjob.h"
#include "coros4map.h"

using namespace IMG;

#define EPSILON    1e-6
#define TILE_SIZE  384
#define DELTA      1e-3

void Coros4Map::loadDir(const QString &path, MapTree &tree)
{
	QDir md(path);
	md.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList ml = md.entryInfoList();
	double min[2], max[2];

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);

		if (fi.isDir())
			loadDir(fi.absoluteFilePath(), tree);
		else {
			IMGData *data = new IMGData(fi.absoluteFilePath(), _polyCache,
			  _pointCache, _demCache, _lock, _demLock);
			if (data->isValid()) {
				min[0] = data->bounds().left();
				min[1] = data->bounds().bottom();
				max[0] = data->bounds().right();
				max[1] = data->bounds().top();

				tree.Insert(min, max, data);

				_dataBounds |= data->bounds();
				_zooms |= data->zooms();
			} else {
				qWarning("%s: %s", qUtf8Printable(data->fileName()),
				  qUtf8Printable(data->errorString()));
				delete data;
			}
		}
	}
}

Coros4Map::Coros4Map(const QString &fileName, QObject *parent)
  : Map(fileName, parent), _projection(PCS::pcs(3857)), _tileRatio(1.0),
  _layer(All), _style(0), _valid(false)
{
	QFileInfo fi(fileName);
	QDir dir(fi.absolutePath());

	QDir mapDir(dir.filePath("Map"));
	if (!mapDir.exists()) {
		_errorString = "Map directory not found";
		return;
	}

	QDir osmDir(mapDir.filePath("OSM"));
	QDir cmDir(mapDir.filePath("CM"));
	loadDir(osmDir.absolutePath(), _osm);
	loadDir(cmDir.absolutePath(), _cm);

	if (!(_dataBounds.isValid() && _zooms.isValid())) {
		_errorString = "No usable map tile found";
		return;
	}

	QString typ(dir.absoluteFilePath(fi.baseName() + ".typ"));
	if (QFileInfo::exists(typ))
		_typ = typ;

	_valid = true;
}

Coros4Map::~Coros4Map()
{
	MapTree::Iterator it;
	for (_osm.GetFirst(it); !_osm.IsNull(it); _osm.GetNext(it))
		delete _osm.GetAt(it);
	for (_cm.GetFirst(it); !_cm.IsNull(it); _cm.GetNext(it))
		delete _cm.GetAt(it);

	delete _style;
}

void Coros4Map::load(const Projection &in, const Projection &out,
  qreal devicelRatio, bool hidpi, int style, int layer)
{
	Q_UNUSED(in);
	Q_UNUSED(hidpi);

	_tileRatio = devicelRatio;
	_projection = out;

	switch (layer) {
		case 1:
			_layer = Landscape;
			break;
		case 2:
			_layer = Topo;
			break;
		default:
			_layer = All;
	}

	const QString *typFile = 0;
	if (style < 0 || style >= styles().size()) {
		if (!_typ.isEmpty())
			typFile = &_typ;
		else if (styles().size())
			typFile = &(styles().first());
	} else
		typFile = &(styles().at(style));

	if (typFile) {
		SubFile typ(*typFile);
		_style = new Style(_tileRatio, &typ);
	} else
		_style = new Style(_tileRatio);

	updateTransform();

	QPixmapCache::clear();
}

void Coros4Map::unload()
{
	cancelJobs(true);

	MapTree::Iterator it;
	for (_osm.GetFirst(it); !_osm.IsNull(it); _osm.GetNext(it))
		_osm.GetAt(it)->clear();
	for (_cm.GetFirst(it); !_cm.IsNull(it); _cm.GetNext(it))
		_cm.GetAt(it)->clear();

	delete _style;
	_style = 0;
}

int Coros4Map::zoomFit(const QSize &size, const RectC &rect)
{
	const Range &zooms = _zooms;

	if (rect.isValid()) {
		RectD pr(rect, _projection, 10);

		_zoom = zooms.min();
		for (int i = zooms.min() + 1; i <= zooms.max(); i++) {
			Transform t(transform(i));
			QRectF r(t.proj2img(pr.topLeft()), t.proj2img(pr.bottomRight()));
			if (size.width() + EPSILON < r.width()
			  || size.height() + EPSILON < r.height())
				break;
			_zoom = i;
		}
	} else
		_zoom = zooms.max();

	updateTransform();

	return _zoom;
}

int Coros4Map::zoomIn()
{
	cancelJobs(false);

	_zoom = qMin(_zoom + 1, _zooms.max());
	updateTransform();
	return _zoom;
}

int Coros4Map::zoomOut()
{
	cancelJobs(false);

	_zoom = qMax(_zoom - 1, _zooms.min());
	updateTransform();
	return _zoom;
}

void Coros4Map::setZoom(int zoom)
{
	_zoom = zoom;
	updateTransform();
}

Transform Coros4Map::transform(int zoom) const
{
	double scale = _projection.isGeographic()
	  ? 360.0 / (1<<zoom) : (2.0 * M_PI * WGS84_RADIUS) / (1<<zoom);
	PointD topLeft(_projection.ll2xy(_dataBounds.topLeft()));
	return Transform(ReferencePoint(PointD(0, 0), topLeft),
	  PointD(scale, scale));
}

void Coros4Map::updateTransform()
{
	_transform = transform(_zoom);

	RectD prect(_dataBounds, _projection);
	_bounds = QRectF(_transform.proj2img(prect.topLeft()),
	  _transform.proj2img(prect.bottomRight()));
	// Adjust the bounds of world maps to avoid problems with wrapping
	if (_dataBounds.left() == -180.0 || _dataBounds.right() == 180.0)
		_bounds.adjust(0.5, 0, -0.5, 0);
}

bool Coros4Map::isRunning(const QString &key) const
{
	for (int i = 0; i < _jobs.size(); i++) {
		const QList<RasterTile> &tiles = _jobs.at(i)->tiles();
		for (int j = 0; j < tiles.size(); j++)
			if (tiles.at(j).key() == key)
				return true;
	}

	return false;
}

void Coros4Map::runJob(IMGJob *job)
{
	_jobs.append(job);

	connect(job, &IMGJob::finished, this, &Coros4Map::jobFinished);
	job->run();
}

void Coros4Map::removeJob(IMGJob *job)
{
	_jobs.removeOne(job);
	job->deleteLater();
}

void Coros4Map::jobFinished(IMGJob *job)
{
	const QList<RasterTile> &tiles = job->tiles();

	for (int i = 0; i < tiles.size(); i++) {
		const RasterTile &mt = tiles.at(i);
		if (!mt.pixmap().isNull())
			QPixmapCache::insert(mt.key(), mt.pixmap());
	}

	removeJob(job);

	emit tilesLoaded();
}

void Coros4Map::cancelJobs(bool wait)
{
	for (int i = 0; i < _jobs.size(); i++)
		_jobs.at(i)->cancel(wait);
}

static bool cb(MapData *data, void *context)
{
	QList<MapData*> *list = (QList<MapData*>*)context;
	list->append(data);
	return true;
}

void Coros4Map::draw(QPainter *painter, const QRectF &rect, Flags flags)
{
	double min[2], max[2];
	QPoint tl(qFloor(rect.left() / TILE_SIZE)
	  * TILE_SIZE, qFloor(rect.top() / TILE_SIZE) * TILE_SIZE);
	QSizeF s(rect.right() - tl.x(), rect.bottom() - tl.y());
	int width = ceil(s.width() / TILE_SIZE);
	int height = ceil(s.height() / TILE_SIZE);

	QList<RasterTile> tiles;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			QPixmap pm;
			QPoint ttl(tl.x() + i * TILE_SIZE, tl.y() + j * TILE_SIZE);
			QString key(path() + "-" + QString::number(_zoom)
			  + "_" + QString::number(ttl.x()) + "_" + QString::number(ttl.y()));

			if (isRunning(key))
				continue;

			if (QPixmapCache::find(key, &pm))
				painter->drawPixmap(ttl, pm);
			else {
				RectD rectD(_transform.img2proj(ttl), _transform.img2proj(
				  QPoint(ttl.x() + TILE_SIZE, ttl.y() + TILE_SIZE)));
				RectC rectC(rectD.toRectC(_projection, 20));
				QList<MapData*> data;

				min[0] = rectC.topLeft().lon();
				min[1] = rectC.bottomRight().lat();
				max[0] = rectC.bottomRight().lon();
				max[1] = rectC.topLeft().lat();

				if (_layer & Landscape)
					_osm.Search(min, max, cb, &data);
				if (_layer & Topo)
					_cm.Search(min, max, cb, &data);

				if (!data.isEmpty())
					tiles.append(RasterTile(_projection, _transform, data,
					  _style, _zoom, QRect(ttl, QSize(TILE_SIZE, TILE_SIZE)),
					  _tileRatio, key, flags & Map::HillShading && _zoom >= 17
					  && _zoom <= 24, false, true));
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
				QPixmapCache::insert(mt.key(), pm);
			}
		} else
			runJob(new IMGJob(tiles));
	}
}

static bool ecb(MapData *data, void *context)
{
	QList<MapData*> *list = (QList<MapData*>*)context;

	if (data->hasDEM())
		list->append(data);

	return true;
}

double Coros4Map::elevation(const Coordinates &c)
{
	double min[2], max[2];
	QList<MapData*> maps;

	min[0] = c.lon();
	min[1] = c.lat();
	max[0] = c.lon();
	max[1] = c.lat();

	_osm.Search(min, max, ecb, &maps);
	if (maps.isEmpty())
		_cm.Search(min, max, ecb, &maps);

	for (int i = 0; i < maps.size(); i++) {
		QList<MapData::Elevation> tiles;
		MapData *map = maps.at(i);

		map->elevations(0, RectC(c, Coordinates(c.lon() + DELTA,
		  c.lat() - DELTA)), map->zooms().max(), &tiles);
		DEMTree tree(tiles);
		double ele = tree.elevation(c);

		if (!std::isnan(ele))
			return ele;
	}

	return Map::elevation(c);
}

QStringList Coros4Map::styles(int &defaultStyle) const
{
	QStringList list;
	list.reserve(styles().size() + (_typ.isEmpty() ? 0 : 1));

	for (int i = 0; i < styles().size(); i++)
		list.append(QFileInfo(styles().at(i)).baseName());

	if (!_typ.isEmpty())
		list.append(QFileInfo(_typ).baseName());

	defaultStyle = _typ.isEmpty() ? 0 : list.size() - 1;

	return list;
}

QStringList Coros4Map::layers(const QString &lang, int &defaultLayer) const
{
	Q_UNUSED(lang);

	defaultLayer = 0;

	return QStringList() << tr("All") << tr("Landscape") << tr("Topo");
}

Map* Coros4Map::create(const QString &path, const Projection &proj, bool *isDir)
{
	Q_UNUSED(proj);

	if (isDir)
		*isDir = true;

	return new Coros4Map(path);
}

Coros4Map::StyleList::StyleList()
{
	QString path(ProgramPaths::styleDir());
	if (path.isEmpty())
		return;

	QDir dir(path);
	QFileInfoList styles(dir.entryInfoList(QStringList("*.typ"), QDir::Files));

	for (int i = 0; i < styles.size(); i++)
		append(styles.at(i).absoluteFilePath());
}

Coros4Map::StyleList &Coros4Map::styles()
{
	static StyleList list;
	return list;
}
