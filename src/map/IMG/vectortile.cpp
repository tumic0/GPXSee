#include "vectortile.h"


static void copyPolys(const RectC &rect, QList<IMG::Poly> *src,
  QList<IMG::Poly> *dst)
{
	for (int i = 0; i < src->size(); i++)
		if (rect.intersects(src->at(i).boundingRect))
			dst->append(src->at(i));
}

static void copyPoints(const RectC &rect, QList<IMG::Point> *src,
  QList<IMG::Point> *dst)
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

SubFile *VectorTile::addFile(IMG *img, SubFile::Type type)
{
	switch (type) {
		case SubFile::TRE:
			_tre = new TREFile(img);
			return _tre;
		case SubFile::RGN:
			_rgn = new RGNFile(img);
			return _rgn;
		case SubFile::LBL:
			_lbl = new LBLFile(img);
			return _lbl;
		case SubFile::NET:
			_net = new NETFile(img);
			return _net;
		case SubFile::NOD:
			_nod = new NODFile(img);
			return _nod;
		case SubFile::GMP:
			_gmp = new SubFile(img);
			return _gmp;
		default:
			return 0;
	}
}

SubFile *VectorTile::addFile(const QString &path, SubFile::Type type)
{
	switch (type) {
		case SubFile::TRE:
			_tre = new TREFile(path);
			return _tre;
		case SubFile::RGN:
			_rgn = new RGNFile(path);
			return _rgn;
		case SubFile::LBL:
			_lbl = new LBLFile(path);
			return _lbl;
		case SubFile::NET:
			_net = new NETFile(path);
			return _net;
		case SubFile::NOD:
			_nod = new NODFile(path);
			return _nod;
		case SubFile::GMP:
			_gmp = new SubFile(path);
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

void VectorTile::polys(const RectC &rect, int bits, bool baseMap,
  QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
  QCache<const SubDiv *, IMG::Polys> *polyCache) const
{
	SubFile::Handle rgnHdl(_rgn), lblHdl(_lbl), netHdl(_net), nodHdl(_nod);

	if (!_rgn->initialized() && !_rgn->init(rgnHdl))
		return;

	QList<SubDiv*> subdivs = _tre->subdivs(rect, bits, baseMap);
	for (int i = 0; i < subdivs.size(); i++) {
		SubDiv *subdiv = subdivs.at(i);

		IMG::Polys *polys = polyCache->object(subdiv);
		if (!polys) {
			quint32 shift = _tre->shift(subdiv->bits());
			QList<IMG::Poly> p, l;

			if (!subdiv->initialized() && !_rgn->subdivInit(rgnHdl, subdiv))
				continue;

			_rgn->polyObjects(rgnHdl, subdiv, RGNFile::Polygon, _lbl, lblHdl,
			  _net, netHdl, &p);
			_rgn->polyObjects(rgnHdl, subdiv, RGNFile::Line, _lbl, lblHdl,
			  _net, netHdl, &l);
			_rgn->extPolyObjects(rgnHdl, subdiv, shift, RGNFile::Polygon, _lbl,
			  lblHdl, &p);
			_rgn->extPolyObjects(rgnHdl, subdiv, shift, RGNFile::Line, _lbl,
			  lblHdl, &l);
			_rgn->links(rgnHdl, subdiv, _net, netHdl, _nod, nodHdl, &l);

			copyPolys(rect, &p, polygons);
			copyPolys(rect, &l, lines);
			polyCache->insert(subdiv, new IMG::Polys(p, l));
		} else {
			copyPolys(rect, &(polys->polygons), polygons);
			copyPolys(rect, &(polys->lines), lines);
		}
	}
}

void VectorTile::points(const RectC &rect, int bits, bool baseMap,
  QList<IMG::Point> *points, QCache<const SubDiv *,
  QList<IMG::Point> > *pointCache) const
{
	SubFile::Handle rgnHdl(_rgn), lblHdl(_lbl);

	if (!_rgn->initialized() && !_rgn->init(rgnHdl))
		return;

	QList<SubDiv*> subdivs = _tre->subdivs(rect, bits, baseMap);
	for (int i = 0; i < subdivs.size(); i++) {
		SubDiv *subdiv = subdivs.at(i);

		QList<IMG::Point> *pl = pointCache->object(subdiv);
		if (!pl) {
			QList<IMG::Point> p;

			if (!subdiv->initialized() && !_rgn->subdivInit(rgnHdl, subdiv))
				continue;

			_rgn->pointObjects(rgnHdl, subdiv, RGNFile::Point, _lbl, lblHdl,
			  &p);
			_rgn->pointObjects(rgnHdl, subdiv, RGNFile::IndexedPoint, _lbl,
			  lblHdl, &p);
			_rgn->extPointObjects(rgnHdl, subdiv, _lbl, lblHdl, &p);

			copyPoints(rect, &p, points);
			pointCache->insert(subdiv, new QList<IMG::Point>(p));
		} else
			copyPoints(rect, pl, points);
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const VectorTile &tile)
{
	dbg.nospace() << "VectorTile(" << tile.bounds() <<")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
