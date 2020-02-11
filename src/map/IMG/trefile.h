#ifndef TREFILE_H
#define TREFILE_H

#include <QVector>
#include <QDebug>
#include <QRect>
#include "common/rectc.h"
#include "common/rtree.h"
#include "subfile.h"

class SubDiv;

class TREFile : public SubFile
{
public:
	TREFile(IMG *img) : SubFile(img) {}
	TREFile(const QString &path) : SubFile(path) {}
	TREFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset) {}
	~TREFile();

	bool init(bool baseMap);
	void clear();

	const RectC &bounds() const {return _bounds;}
	QList<SubDiv*> subdivs(const RectC &rect, int bits, bool baseMap);
	quint32 shift(quint8 bits) const
	  {return (bits == _levels.last().bits) ? (_flags >> 0xb) & 7 : 0;}

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
	bool parsePoly(Handle hdl, quint32 pos, const QMap<int, int> &level2bits,
	  QMap<quint32, int> &map);
	bool parsePoints(Handle hdl, quint32 pos, const QMap<int, int> &level2bits);

	friend QDebug operator<<(QDebug dbg, const MapLevel &level);

	RectC _bounds;
	QVector<MapLevel> _levels;
	quint32 _subdivOffset;
	Extended _extended;
	int _firstLevel;
	quint32 _flags;
	bool _isBaseMap;

	QMap<int, SubDivTree*> _subdivs;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const TREFile::MapLevel &level);
#endif // QT_NO_DEBUG

#endif // TREFILE_H
