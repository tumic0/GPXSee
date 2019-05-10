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
	TREFile(IMG *img, quint32 size) : SubFile(img, size) {}
	~TREFile();

	bool init();

	const RectC &bounds() const {return _bounds;}
	const QList<int> bits() const {return _subdivs.keys();}

	QList<SubDiv*> subdivs(const RectC &rect, int bits) const;

private:
	typedef RTree<SubDiv*, double, 2> SubDivTree;

	bool parsePoly(Handle hdl, quint32 pos, const QMap<int, int> &level2bits,
	  QMap<quint32, int> &map);
	bool parsePoints(Handle hdl, quint32 pos, const QMap<int, int> &level2bits);

	RectC _bounds;
	QMap<int, SubDivTree*> _subdivs;
};

#endif // TREFILE_H
