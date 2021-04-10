#ifndef IMG_VECTORTILE_H
#define IMG_VECTORTILE_H

#include "trefile.h"
#include "rgnfile.h"
#include "lblfile.h"
#include "netfile.h"
#include "nodfile.h"

namespace IMG {

class VectorTile {
public:
	VectorTile()
	  : _tre(0), _rgn(0), _lbl(0), _net(0), _nod(0), _gmp(0), _loaded(0) {}
	~VectorTile()
	{
		delete _tre; delete _rgn; delete _lbl; delete _net; delete _nod;
		delete _gmp;
	}

	bool init();
	void markAsBasemap() {_tre->markAsBasemap();}
	void clear();

	const RectC &bounds() const {return _tre->bounds();}
	Range zooms() const {return _tre->zooms();}

	SubFile *file(SubFile::Type type);

	void polys(const RectC &rect, int bits, bool baseMap,
	  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
	  QCache<const SubDiv *, MapData::Polys> *polyCache);
	void points(const RectC &rect, int bits, bool baseMap,
	  QList<MapData::Point> *points, QCache<const SubDiv*,
	  QList<MapData::Point> > *pointCache);

	static bool isTileFile(SubFile::Type type)
	{
		return (type == SubFile::TRE || type == SubFile::LBL
		  || type == SubFile::RGN || type == SubFile::NET
		  || type == SubFile::NOD || type == SubFile::GMP);
	}

	template<typename T>
	SubFile *addFile(T *container, SubFile::Type type)
	{
		switch (type) {
			case SubFile::TRE:
				_tre = new TREFile(container);
				return _tre;
			case SubFile::RGN:
				_rgn = new RGNFile(container);
				return _rgn;
			case SubFile::LBL:
				_lbl = new LBLFile(container);
				return _lbl;
			case SubFile::NET:
				_net = new NETFile(container);
				return _net;
			case SubFile::NOD:
				_nod = new NODFile(container);
				return _nod;
			case SubFile::GMP:
				_gmp = new SubFile(container);
				return _gmp;
			default:
				return 0;
		}
	}

private:
	bool initGMP();
	bool load(SubFile::Handle &rgnHdl, SubFile::Handle &lblHdl,
	  SubFile::Handle &netHdl, SubFile::Handle &nodHdl);

	TREFile *_tre;
	RGNFile *_rgn;
	LBLFile *_lbl;
	NETFile *_net;
	NODFile *_nod;
	SubFile *_gmp;

	int _loaded;
};

}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const IMG::VectorTile &tile);
#endif // QT_NO_DEBUG

#endif // IMG_VECTORTILE_H
