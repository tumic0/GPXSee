#include "vectortile.h"

using namespace IMG;

static void copyPolys(const RectC &rect, const QList<MapData::Poly> *src,
  QList<MapData::Poly> *dst)
{
	for (int i = 0; i < src->size(); i++)
		if (rect.intersects(src->at(i).boundingRect))
			dst->append(src->at(i));
}

static void copyPoints(const RectC &rect, const QList<MapData::Point> *src,
  QList<MapData::Point> *dst)
{
	for (int j = 0; j < src->size(); j++)
		if (rect.contains(src->at(j).coordinates))
			dst->append(src->at(j));
}

SubFile *VectorTile::file(SubFile::Type type)
{
	switch (type) {
		case SubFile::TRE:
			return _tre;
		case SubFile::RGN:
			return _rgn;
		case SubFile::LBL:
			return _lbl;
		case SubFile::NET:
			return _net;
		case SubFile::NOD:
			return _nod;
		case SubFile::DEM:
			return _dem;
		case SubFile::GMP:
			return _gmp;
		default:
			return 0;
	}
}

bool VectorTile::init(QFile *file)
{
	if (_gmp && !initGMP(file))
		return false;

	if (!(_tre && _tre->init(file) && _rgn))
		return false;

	return true;
}

bool VectorTile::initGMP(QFile *file)
{
	SubFile::Handle hdl(_gmp, file);
	quint32 tre, rgn, lbl, net, nod, dem;

	if (_tre || _rgn || _lbl || _net || _nod || _dem)
		return false;

	if (!(_gmp->seek(hdl, 0x19) && _gmp->readUInt32(hdl, tre)
	  && _gmp->readUInt32(hdl, rgn) && _gmp->readUInt32(hdl, lbl)
	  && _gmp->readUInt32(hdl, net) && _gmp->readUInt32(hdl, nod)
	  && _gmp->readUInt32(hdl, dem)))
		return false;

	_tre = tre ? new TREFile(_gmp, tre) : 0;
	_rgn = rgn ? new RGNFile(_gmp, rgn) : 0;
	_lbl = lbl ? new LBLFile(_gmp, lbl) : 0;
	_net = net ? new NETFile(_gmp, net) : 0;
	_nod = nod ? new NODFile(_gmp, nod) : 0;
	_dem = dem ? new DEMFile(_gmp, dem) : 0;

	return true;
}

bool VectorTile::load(SubFile::Handle &rgnHdl, SubFile::Handle &lblHdl,
  SubFile::Handle &netHdl, SubFile::Handle &nodHdl)
{
	_loaded = -1;

	if (!_rgn->load(rgnHdl))
		return false;
	if (_lbl && !_lbl->load(lblHdl, _rgn, rgnHdl))
		return false;
	if (_net && !_net->load(netHdl, _rgn, rgnHdl))
		return false;
	if (_nod && !_nod->load(nodHdl))
		return false;

	_loaded = 1;

	return true;
}

bool VectorTile::loadDem(SubFile::Handle &hdl)
{
	_demLoaded = -1;

	if (!_dem || !_dem->load(hdl))
		return false;

	_demLoaded = 1;

	return true;
}

void VectorTile::clear()
{
	_tre->clear();
	_rgn->clear();
	if (_lbl)
		_lbl->clear();
	if (_net)
		_net->clear();
	if (_dem)
		_dem->clear();

	_loaded = 0;
	_demLoaded = 0;
}

void VectorTile::polys(QFile *file, const RectC &rect, const Zoom &zoom,
  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
  MapData::PolyCache *cache, QMutex *cacheLock)
{
	SubFile::Handle *rgnHdl = 0, *lblHdl = 0, *netHdl = 0, *nodHdl = 0,
	  *nodHdl2 = 0;

	_lock.lock();

	if (_loaded < 0) {
		_lock.unlock();
		return;
	}

	if (!_loaded) {
		rgnHdl = new SubFile::Handle(_rgn, file);
		lblHdl = new SubFile::Handle(_lbl, file);
		netHdl = new SubFile::Handle(_net, file);
		nodHdl = new SubFile::Handle(_nod, file);

		if (!load(*rgnHdl, *lblHdl, *netHdl, *nodHdl)) {
			_lock.unlock();
			delete rgnHdl; delete lblHdl; delete netHdl; delete nodHdl;
			return;
		}
	}

	QList<SubDiv*> subdivs = _tre->subdivs(file, rect, zoom);

	cacheLock->lock();

	for (int i = 0; i < subdivs.size(); i++) {
		SubDiv *subdiv = subdivs.at(i);

		MapData::Polys *polys = cache->object(subdiv);
		if (!polys) {
			cacheLock->unlock();

			quint32 shift = _tre->shift(subdiv->bits());

			if (!rgnHdl) {
				rgnHdl = new SubFile::Handle(_rgn, file);
				lblHdl = new SubFile::Handle(_lbl, file);
				netHdl = new SubFile::Handle(_net, file);
			}

			if (!subdiv->initialized() && !_rgn->subdivInit(*rgnHdl, subdiv)) {
				cacheLock->lock();
				continue;
			}

			polys = new MapData::Polys();

			_rgn->polyObjects(*rgnHdl, subdiv, RGNFile::Polygon, _lbl, *lblHdl,
			  _net, *netHdl, &polys->polygons);
			_rgn->polyObjects(*rgnHdl, subdiv, RGNFile::Line, _lbl, *lblHdl,
			  _net, *netHdl, &polys->lines);
			_rgn->extPolyObjects(*rgnHdl, subdiv, shift, RGNFile::Polygon, _lbl,
			  *lblHdl, &polys->polygons);
			_rgn->extPolyObjects(*rgnHdl, subdiv, shift, RGNFile::Line, _lbl,
			  *lblHdl, &polys->lines);

			if (_net && _net->hasLinks()) {
				if (!nodHdl)
					nodHdl = new SubFile::Handle(_nod, file);
				if (!nodHdl2)
					nodHdl2 = new SubFile::Handle(_nod, file);
				_rgn->links(*rgnHdl, subdiv, shift, _net, *netHdl, _nod, *nodHdl,
				  *nodHdl2, _lbl, *lblHdl, &polys->lines);
			}

			copyPolys(rect, &polys->polygons, polygons);
			if (lines)
				copyPolys(rect, &polys->lines, lines);

			cacheLock->lock();
			cache->insert(subdiv, polys);
		} else {
			copyPolys(rect, &polys->polygons, polygons);
			if (lines)
				copyPolys(rect, &polys->lines, lines);
		}
	}

	cacheLock->unlock();
	_lock.unlock();

	delete rgnHdl; delete lblHdl; delete netHdl; delete nodHdl; delete nodHdl2;
}

void VectorTile::points(QFile *file, const RectC &rect, const Zoom &zoom,
  QList<MapData::Point> *points, MapData::PointCache *cache, QMutex *cacheLock)
{
	SubFile::Handle *rgnHdl = 0, *lblHdl = 0;

	_lock.lock();

	if (_loaded < 0) {
		_lock.unlock();
		return;
	}

	if (!_loaded) {
		rgnHdl = new SubFile::Handle(_rgn, file);
		lblHdl = new SubFile::Handle(_lbl, file);
		SubFile::Handle nodHdl(_nod, file);
		SubFile::Handle netHdl(_net, file);

		if (!load(*rgnHdl, *lblHdl, netHdl, nodHdl)) {
			_lock.unlock();
			delete rgnHdl; delete lblHdl;
			return;
		}
	}

	QList<SubDiv*> subdivs = _tre->subdivs(file, rect, zoom);

	cacheLock->lock();

	for (int i = 0; i < subdivs.size(); i++) {
		SubDiv *subdiv = subdivs.at(i);

		QList<MapData::Point> *pl = cache->object(subdiv);
		if (!pl) {
			cacheLock->unlock();

			if (!rgnHdl) {
				rgnHdl = new SubFile::Handle(_rgn, file);
				lblHdl = new SubFile::Handle(_lbl, file);
			}

			if (!subdiv->initialized() && !_rgn->subdivInit(*rgnHdl, subdiv)) {
				cacheLock->lock();
				continue;
			}

			pl = new QList<MapData::Point>;

			_rgn->pointObjects(*rgnHdl, subdiv, RGNFile::Point, _lbl, *lblHdl,
			  pl);
			_rgn->pointObjects(*rgnHdl, subdiv, RGNFile::IndexedPoint, _lbl,
			  *lblHdl, pl);
			_rgn->extPointObjects(*rgnHdl, subdiv, _lbl, *lblHdl, pl);

			copyPoints(rect, pl, points);

			cacheLock->lock();
			cache->insert(subdiv, pl);
		} else
			copyPoints(rect, pl, points);
	}

	cacheLock->unlock();
	_lock.unlock();

	delete rgnHdl; delete lblHdl;
}

void VectorTile::elevations(QFile *file, const RectC &rect, const Zoom &zoom,
  QList<MapData::Elevation> *elevations, MapData::ElevationCache *cache,
  QMutex *cacheLock)
{
	SubFile::Handle *hdl = 0;

	_demLock.lock();

	if (_demLoaded < 0) {
		_demLock.unlock();
		return;
	}

	if (!_demLoaded) {
		hdl = new SubFile::Handle(_dem, file);

		if (!loadDem(*hdl)) {
			_demLock.unlock();
			delete hdl;
			return;
		}
	}

	// Shift the DEM level to get better data then what the map defines for
	// the given zoom (we prefer rendering quality rather than speed). For
	// maps with a single level this has no effect.
	int level = qMax(0, _dem->level(zoom) - 1);
	QList<const DEMTile*> tiles(_dem->tiles(rect, level));

	cacheLock->lock();

	for (int i = 0; i < tiles.size(); i++) {
		const DEMTile *tile = tiles.at(i);

		MapData::Elevation *el = cache->object(tile);
		if (!el) {
			cacheLock->unlock();

			if (!hdl)
				hdl = new SubFile::Handle(_dem, file);

			el = _dem->elevations(*hdl, level, tile);
			if (!el->m.isNull())
				elevations->append(*el);

			cacheLock->lock();
			cache->insert(tile, el);
		} else {
			if (!el->m.isNull())
				elevations->append(*el);
		}
	}

	cacheLock->unlock();
	_demLock.unlock();

	delete hdl;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const VectorTile &tile)
{
	dbg.nospace() << "VectorTile(" << tile.bounds() <<")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
