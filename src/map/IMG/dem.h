#ifndef IMG_DEM_H
#define IMG_DEM_H

#include "common/rtree.h"
#include "mapdata.h"

namespace IMG {

class DEM {
public:

	typedef RTree<const MapData::Elevation*, double, 2> DEMTRee;

	static void buildTree(const QList<MapData::Elevation> &tiles, DEMTRee &tree);
	static void searchTree(const DEMTRee &tree, const Coordinates &c,
	  double &ele);

private:
	struct ElevationCTX {
		ElevationCTX(const DEMTRee &tree, const Coordinates &c, double &ele)
		  : tree(tree), c(c), ele(ele) {}

		const DEMTRee &tree;
		const Coordinates &c;
		double &ele;
	};

	struct EdgeCTX {
		EdgeCTX(const Coordinates &c, double &ele) : c(c), ele(ele) {}

		const Coordinates &c;
		double &ele;
	};

	static double edge(const DEMTRee &tree, const Coordinates &c);
	static double elevation(const DEMTRee &tree, const MapData::Elevation *e,
	  const Coordinates &c);
	static bool elevationCb(const MapData::Elevation *e, void *context);
	static bool edgeCb(const MapData::Elevation *e, void *context);
};

}

#endif // IMG_ELEVATIONTREE_H
