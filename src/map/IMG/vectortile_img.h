#ifndef IMG_VECTORTILE_H
#define IMG_VECTORTILE_H

#include "trefile.h"
#include "rgnfile.h"
#include "lblfile.h"
#include "netfile.h"
#include "nodfile.h"
#include "demfile.h"

namespace IMG {

class VectorTile {
public:
	VectorTile()
	  : _tre(0), _rgn(0), _lbl(0), _net(0), _nod(0), _dem(0), _gmp(0),
		_loaded(0), _demLoaded(0) {}
	~VectorTile()
	{
		delete _tre; delete _rgn; delete _lbl; delete _net; delete _nod;
		delete _dem; delete _gmp;
	}

	bool init(QFile *file = 0);
	void clear();

	const RectC &bounds() const {return _tre->bounds();}
	QVector<Zoom> zooms() const {return _tre->zooms();}
	bool hasDem() const {return _dem != 0;}

	SubFile *file(SubFile::Type type);

	void polys(QFile *file, const RectC &rect, const Zoom &zoom,
	  QList<MapData::Poly> *polygons, QList<MapData::Poly> *lines,
	  MapData::PolyCache *cache, QMutex *cacheLock);
	void points(QFile *file, const RectC &rect, const Zoom &zoom,
	  QList<MapData::Point> *points, MapData::PointCache *cache,
	  QMutex *cacheLock);
	void elevations(QFile *file, const RectC &rect, const Zoom &zoom,
	  QList<MapData::Elevation> *elevations, MapData::ElevationCache *cache,
	  QMutex *cacheLock);

	static bool isTileFile(SubFile::Type type)
	{
		return (type == SubFile::TRE || type == SubFile::LBL
		  || type == SubFile::RGN || type == SubFile::NET
		  || type == SubFile::NOD || type == SubFile::DEM
		  || type == SubFile::GMP);
	}

	template<typename T>
	SubFile *addFile(T container, SubFile::Type type)
	{
		switch (type) {
			case SubFile::TRE:
				if (_tre)
					return 0;
				_tre = new TREFile(container);
				return _tre;
			case SubFile::RGN:
				if (_rgn)
					return 0;
				_rgn = new RGNFile(container);
				return _rgn;
			case SubFile::LBL:
				if (_lbl)
					return 0;
				_lbl = new LBLFile(container);
				return _lbl;
			case SubFile::NET:
				if (_net)
					return 0;
				_net = new NETFile(container);
				return _net;
			case SubFile::NOD:
				if (_nod)
					return 0;
				_nod = new NODFile(container);
				return _nod;
			case SubFile::DEM:
				if (_dem)
					return 0;
				_dem = new DEMFile(container);
				return _dem;
			case SubFile::GMP:
				_gmp = new SubFile(container);
				return _gmp;
			default:
				return 0;
		}
	}

private:
	bool initGMP(QFile *file);
	bool load(SubFile::Handle &rgnHdl, SubFile::Handle &lblHdl,
	  SubFile::Handle &netHdl, SubFile::Handle &nodHdl);
	bool loadDem(SubFile::Handle &demHdl);

	TREFile *_tre;
	RGNFile *_rgn;
	LBLFile *_lbl;
	NETFile *_net;
	NODFile *_nod;
	DEMFile *_dem;
	SubFile *_gmp;

	int _loaded, _demLoaded;
	QMutex _lock, _demLock;
};

}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const IMG::VectorTile &tile);
#endif // QT_NO_DEBUG

#endif // IMG_VECTORTILE_H
