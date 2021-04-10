#ifndef IMG_TREFILE_H
#define IMG_TREFILE_H

#include <QVector>
#include <QDebug>
#include <QRect>
#include "common/rectc.h"
#include "common/rtree.h"
#include "subfile.h"

namespace IMG {

class SubDiv;

class TREFile : public SubFile
{
public:
	TREFile(const IMGData *img) : SubFile(img) {}
	TREFile(const QString *path) : SubFile(path) {}
	TREFile(const SubFile *gmp, quint32 offset) : SubFile(gmp, offset) {}
	~TREFile();

	bool init();
	void markAsBasemap() {_isBaseMap = true;}
	void clear();

	const RectC &bounds() const {return _bounds;}
	QList<SubDiv*> subdivs(const RectC &rect, int bits, bool baseMap);
	quint32 shift(quint8 bits) const
	  {return (bits == _levels.last().bits) ? (_flags >> 0xb) & 7 : 0;}
	Range zooms() const
	  {return Range(_levels.at(_firstLevel).bits, _levels.last().bits);}

private:
	struct MapLevel {
		quint8 level;
		quint8 bits;
		quint16 subdivs;
	};
	struct Extended {
		quint32 offset;
		quint32 size;
		quint16 itemSize;

		Extended() : offset(0), size(0), itemSize(0) {}
	};
	typedef RTree<SubDiv*, double, 2> SubDivTree;

	bool load(int idx);
	int level(int bits, bool baseMap);
	int readExtEntry(Handle &hdl, quint32 &polygons, quint32 &lines,
	  quint32 &points);

	RectC _bounds;
	QVector<MapLevel> _levels;
	quint32 _subdivOffset;
	Extended _extended;
	int _firstLevel;
	quint32 _flags;
	bool _isBaseMap;

	QMap<int, SubDivTree*> _subdivs;
};

}

#endif // IMG_TREFILE_H
