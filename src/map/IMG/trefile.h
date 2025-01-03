#ifndef IMG_TREFILE_H
#define IMG_TREFILE_H

#include <QVector>
#include <QDebug>
#include <QRect>
#include "common/rectc.h"
#include "common/rtree.h"
#include "section.h"
#include "subfile.h"

namespace IMG {

class SubDiv;

class TREFile : public SubFile
{
public:
	TREFile(const IMGData *img)
	  : SubFile(img), _flags(0), _extItemSize(0) {}
	TREFile(const QString &path)
	  : SubFile(path), _flags(0), _extItemSize(0) {}
	TREFile(const SubFile *gmp, quint32 offset)
	  : SubFile(gmp, offset), _flags(0), _extItemSize(0) {}
	~TREFile();

	bool init(QFile *file);
	void clear();

	const RectC &bounds() const {return _bounds;}
	QList<SubDiv*> subdivs(QFile *file, const RectC &rect, const Zoom &zoom);
	quint32 shift(quint8 bits) const
	  {return (bits == _levels.last().bits) ? (_flags >> 0xb) & 7 : 0;}
	QVector<Zoom> zooms() const;

private:
	struct MapLevel {
		quint8 level;
		quint8 bits;
		quint16 subdivs;
	};
	typedef RTree<SubDiv*, double, 2> SubDivTree;

	bool load(QFile *file, int idx);
	const SubDivTree *subdivs(QFile *file, const Zoom &zoom);
	int readExtEntry(Handle &hdl, quint32 &polygons, quint32 &lines,
	  quint32 &points);

	RectC _bounds;
	QVector<MapLevel> _levels;
	Section _subdivSec, _extSec;
	quint32 _flags;
	quint16 _extItemSize;
	int _firstLevel;

	QMap<int, SubDivTree*> _subdivs;
};

}

#endif // IMG_TREFILE_H
