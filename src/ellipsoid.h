#ifndef ELLIPSOID_H
#define ELLIPSOID_H

class QString;

class Ellipsoid
{
public:
	Ellipsoid();
	Ellipsoid(const QString &datum);

	double radius() const {return _radius;}
	double flattening() const {return _flattening;}

	double q(double b) const;
	double iq(double q) const;
	double nradius(double phi) const;

private:
	double _radius;
	double _flattening;
};

#endif // ELLIPSOID_H
