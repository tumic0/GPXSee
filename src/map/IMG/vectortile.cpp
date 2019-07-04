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
		default:
			return 0;
	}
}

SubFile *VectorTile::addFile(IMG *img, SubFile::Type type, quint32 size)
{
	switch (type) {
		case SubFile::TRE:
			_tre = new TREFile(img, size);
			return _tre;
		case SubFile::RGN:
			_rgn = new RGNFile(img, size);
			return _rgn;
		case SubFile::LBL:
			_lbl = new LBLFile(img, size);
			return _lbl;
		case SubFile::NET:
			_net = new NETFile(img, size);
			return _net;
		default:
			return 0;
	}
}

bool VectorTile::init()
{
	if (!(_tre && _tre->isValid() && _tre->init() && _rgn
	  && _rgn->isValid()))
		return false;
	if (_lbl && !_lbl->isValid())
		return false;
	if (_net && !_net->isValid())
		return false;

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
	dbg.nospace() << "VectorTile(";
	if (tile._tre)
		dbg << "TRE: " << *(tile._tre);
	if (tile._rgn)
		dbg << ", RGN: " << *(tile._rgn);
	if (tile._lbl)
		dbg << ", LBL: " << *(tile._lbl);
	if (tile._net)
		dbg << ", NET: " << *(tile._net);
	dbg << ")";

	return dbg.space();
}
#endif // QT_NO_DEBUG
