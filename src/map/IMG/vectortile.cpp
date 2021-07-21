#include "vectortile.h"

using namespace IMG;

static void copyPolys(const RectC &rect, QList<MapData::Poly> *src,
  QList<MapData::Poly> *dst)
{
	for (int i = 0; i < src->size(); i++)
		if (rect.intersects(src->at(i).boundingRect))
			dst->append(src->at(i));
}

static void copyPoints(const RectC &rect, QList<MapData::Point> *src,
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
		case SubFile::GMP:
			return _gmp;
		default:
			return 0;
	}
}

bool VectorTile::init()
{
	if (_gmp && !initGMP())
		return false;

	if (!(_tre && _tre->init() && _rgn))
		return false;

	return true;
}

bool VectorTile::initGMP()
{
	SubFile::Handle hdl(_gmp);
	quint32 tre, rgn, lbl, net, nod;

	if (!(_gmp->seek(hdl, 0x19) && _gmp->readUInt32(hdl, tre)
	  && _gmp->readUInt32(hdl, rgn) && _gmp->readUInt32(hdl, lbl)
	  && _gmp->readUInt32(hdl, net) && _gmp->readUInt32(hdl, nod)))
		return false;

	_tre = tre ? new TREFile(_gmp, tre) : 0;
	_rgn = rgn ? new RGNFile(_gmp, rgn) : 0;
	_lbl = lbl ? new LBLFile(_gmp, lbl) : 0;
	_net = net ? new NETFile(_gmp, net) : 0;
	_nod = nod ? new NODFile(_gmp, nod) : 0;

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

void VectorTile::clear()
{
	_tre->clear();
	_rgn->clear();
	if (_lbl)
		_lbl->clear();
	if (_net)
		_net->clear();

	_loaded = 0;
}

void VectorTile::polys(const RectC &rect, int bits, bool baseMap,
  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
  QCache<const SubDiv *, MapData::Polys> *polyCache)
{
	SubFile::Handle *rgnHdl = 0, *lblHdl = 0, *netHdl = 0, *nodHdl = 0,
	  *nodHdl2 = 0;

	if (_loaded < 0)
		return;
	if (!_loaded) {
		rgnHdl = new SubFile::Handle(_rgn);
		lblHdl = new SubFile::Handle(_lbl);
		netHdl = new SubFile::Handle(_net);
		nodHdl = new SubFile::Handle(_nod);

		if (!load(*rgnHdl, *lblHdl, *netHdl, *nodHdl)) {
			delete rgnHdl; delete lblHdl; delete netHdl; delete nodHdl;
			return;
		}
	}

	QList<SubDiv*> subdivs = _tre->subdivs(rect, bits, baseMap);
	for (int i = 0; i < subdivs.size(); i++) {
		SubDiv *subdiv = subdivs.at(i);

		MapData::Polys *polys = polyCache->object(subdiv);
		if (!polys) {
			quint32 shift = _tre->shift(subdiv->bits());
			QList<MapData::Poly> p, l;

			if (!rgnHdl) {
				rgnHdl = new SubFile::Handle(_rgn);
				lblHdl = new SubFile::Handle(_lbl);
				netHdl = new SubFile::Handle(_net);
			}

			if (!subdiv->initialized() && !_rgn->subdivInit(*rgnHdl, subdiv))
				continue;

			_rgn->polyObjects(*rgnHdl, subdiv, RGNFile::Polygon, _lbl, *lblHdl,
			  _net, *netHdl, &p);
			_rgn->polyObjects(*rgnHdl, subdiv, RGNFile::Line, _lbl, *lblHdl,
			  _net, *netHdl, &l);
			_rgn->extPolyObjects(*rgnHdl, subdiv, shift, RGNFile::Polygon, _lbl,
			  *lblHdl, &p);
			_rgn->extPolyObjects(*rgnHdl, subdiv, shift, RGNFile::Line, _lbl,
			  *lblHdl, &l);

			if (_net && _net->hasLinks()) {
				if (!nodHdl)
					nodHdl = new SubFile::Handle(_nod);
				if (!nodHdl2)
					nodHdl2 = new SubFile::Handle(_nod);
				_rgn->links(*rgnHdl, subdiv, shift, _net, *netHdl, _nod, *nodHdl,
				  *nodHdl2, _lbl, *lblHdl, &l);
			}

			copyPolys(rect, &p, polygons);
			copyPolys(rect, &l, lines);
			polyCache->insert(subdiv, new MapData::Polys(p, l));
		} else {
			copyPolys(rect, &(polys->polygons), polygons);
			copyPolys(rect, &(polys->lines), lines);
		}
	}

	delete rgnHdl; delete lblHdl; delete netHdl; delete nodHdl; delete nodHdl2;
}

void VectorTile::points(const RectC &rect, int bits, bool baseMap,
  QList<MapData::Point> *points, QCache<const SubDiv *,
  QList<MapData::Point> > *pointCache)
{
	SubFile::Handle *rgnHdl = 0, *lblHdl = 0;

	if (_loaded < 0)
		return;
	if (!_loaded) {
		rgnHdl = new SubFile::Handle(_rgn);
		lblHdl = new SubFile::Handle(_lbl);
		SubFile::Handle nodHdl(_nod);
		SubFile::Handle netHdl(_net);

		if (!load(*rgnHdl, *lblHdl, netHdl, nodHdl)) {
			delete rgnHdl; delete lblHdl;
			return;
		}
	}

	QList<SubDiv*> subdivs = _tre->subdivs(rect, bits, baseMap);
	for (int i = 0; i < subdivs.size(); i++) {
		SubDiv *subdiv = subdivs.at(i);

		QList<MapData::Point> *pl = pointCache->object(subdiv);
		if (!pl) {
			QList<MapData::Point> p;

			if (!rgnHdl) {
				rgnHdl = new SubFile::Handle(_rgn);
				lblHdl = new SubFile::Handle(_lbl);
			}

			if (!subdiv->initialized() && !_rgn->subdivInit(*rgnHdl, subdiv))
				continue;

			_rgn->pointObjects(*rgnHdl, subdiv, RGNFile::Point, _lbl, *lblHdl,
			  &p);
			_rgn->pointObjects(*rgnHdl, subdiv, RGNFile::IndexedPoint, _lbl,
			  *lblHdl, &p);
			_rgn->extPointObjects(*rgnHdl, subdiv, _lbl, *lblHdl, &p);

			copyPoints(rect, &p, points);
			pointCache->insert(subdiv, new QList<MapData::Point>(p));
		} else
			copyPoints(rect, pl, points);
	}

	delete rgnHdl; delete lblHdl;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const VectorTile &tile)
{
	dbg.nospace() << "VectorTile(" << tile.bounds() <<")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
