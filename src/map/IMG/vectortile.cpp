#include "vectortile.h"

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
		case SubFile::GMP:
			_gmp = new SubFile(img);
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
	SubFile::Handle hdl;
	quint32 tre, rgn, lbl, net;

	if (!(_gmp->seek(hdl, 0x19) && _gmp->readUInt32(hdl, tre)
	  && _gmp->readUInt32(hdl, rgn) && _gmp->readUInt32(hdl, lbl)
	  && _gmp->readUInt32(hdl, net)))
		return false;

	_tre = new TREFile(_gmp, tre);
	_rgn = new RGNFile(_gmp, rgn);
	_lbl = new LBLFile(_gmp, lbl);
	_net = new NETFile(_gmp, net);

	return true;
}

void VectorTile::objects(const RectC &rect, int bits,
  QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
  QList<IMG::Point> *points) const
{
	QList<SubDiv*> subdivs = _tre->subdivs(rect, bits);
	for (int i = 0; i < subdivs.size(); i++) {
		_rgn->objects(rect, subdivs.at(i), _lbl, _net, polygons, lines, points);
		_rgn->extObjects(rect, subdivs.at(i), _lbl, polygons, lines,
		  points);
	}
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const VectorTile &tile)
{
	dbg.nospace() << "VectorTile(" << tile.bounds() <<")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
