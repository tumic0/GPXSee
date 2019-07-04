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
	void clear();

	const RectC &bounds() const {return _bounds;}
	QList<SubDiv*> subdivs(const RectC &rect, int bits);

private:
	typedef RTree<SubDiv*, double, 2> SubDivTree;

	bool load();
	int level(int bits);
	bool parsePoly(Handle hdl, quint32 pos, const QMap<int, int> &level2bits,
	  QMap<quint32, int> &map);
	bool parsePoints(Handle hdl, quint32 pos, const QMap<int, int> &level2bits);

	RectC _bounds;
	QList<int> _levels;
	QMap<int, SubDivTree*> _subdivs;
};

#endif // TREFILE_H
