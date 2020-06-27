#ifndef VECTORTILE_H
#define VECTORTILE_H

#include "trefile.h"
#include "trefile.h"
#include "rgnfile.h"
#include "lblfile.h"
#include "netfile.h"
#include "nodfile.h"

class VectorTile {
public:
	VectorTile() : _tre(0), _rgn(0), _lbl(0), _net(0), _nod(0), _gmp(0) {}
	~VectorTile()
	{
		delete _tre; delete _rgn; delete _lbl; delete _net; delete _nod;
		delete _gmp;
	}

	bool init();
	void markAsBasemap() {_tre->markAsBasemap();}
	void clear() {_tre->clear();}

	const RectC &bounds() const {return _tre->bounds();}
	Range zooms() const {return _tre->zooms();}

	SubFile *file(SubFile::Type type);
	SubFile *addFile(IMG *img, SubFile::Type type);
	SubFile *addFile(const QString &path, SubFile::Type type);

	void polys(const RectC &rect, int bits, bool baseMap,
	  QList<IMG::Poly> *polygons, QList<IMG::Poly> *lines,
	  QCache<const SubDiv *, IMG::Polys> *polyCache) const;
	void points(const RectC &rect, int bits, bool baseMap,
	  QList<IMG::Point> *points, QCache<const SubDiv*,
	  QList<IMG::Point> > *pointCache) const;

	static bool isTileFile(SubFile::Type type)
	{
		return (type == SubFile::TRE || type == SubFile::LBL
		  || type == SubFile::RGN || type == SubFile::NET
		  || type == SubFile::NOD || type == SubFile::GMP);
	}

private:
	bool initGMP();

	TREFile *_tre;
	RGNFile *_rgn;
	LBLFile *_lbl;
	NETFile *_net;
	NODFile *_nod;
	SubFile *_gmp;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const VectorTile &tile);
#endif // QT_NO_DEBUG

#endif // VECTORTILE_H
