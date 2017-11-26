#ifndef PROJECTION_H
#define PROJECTION_H

#include <QPointF>
#include <QString>
#include "common/coordinates.h"

class Ellipsoid;

class Projection {
public:
	struct Setup {
		double latitudeOrigin;
		double longitudeOrigin;
		double scale;
		double falseEasting;
		double falseNorthing;
		double standardParallel1;
		double standardParallel2;
		int zone;
	};

	virtual ~Projection() {}

	virtual QPointF ll2xy(const Coordinates &c) const = 0;
	virtual Coordinates xy2ll(const QPointF &p) const = 0;

	static Projection *projection(const QString &name,
	  const Ellipsoid &ellipsoid, const Setup &setup);
	static const QString &errorString();

private:
	static QString _errorString;
};

#endif // PROJECTION_H
