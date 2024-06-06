#ifndef IMG_DEMTREE_H
#define IMG_DEMTREE_H

#include "common/rtree.h"
#include "mapdata.h"

namespace IMG {

class DEMTree {
public:
	DEMTree(const QList<MapData::Elevation> &tiles);

	double elevation(const Coordinates &c) const;

private:
	typedef RTree<const MapData::Elevation*, double, 2> Tree;

	struct ElevationCTX {
		ElevationCTX(const Tree &tree, const Coordinates &c, double &ele)
		  : tree(tree), c(c), ele(ele) {}

		const Tree &tree;
		const Coordinates &c;
		double &ele;
	};

	struct EdgeCTX {
		EdgeCTX(const Coordinates &c, double &ele) : c(c), ele(ele) {}

		const Coordinates &c;
		double &ele;
	};

	static double edge(const Tree &tree, const Coordinates &c);
	static double elevation(const Tree &tree, const MapData::Elevation *e,
	  const Coordinates &c);
	static bool elevationCb(const MapData::Elevation *e, void *context);
	static bool edgeCb(const MapData::Elevation *e, void *context);

	Tree _tree;
};

}

#endif // IMG_DEMTREE_H
