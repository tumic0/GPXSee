#ifndef ELLIPSOID_H
#define ELLIPSOID_H

class Ellipsoid
{
public:
	enum Name {
		Clarke1866,
		GRS80,
		International1924,
		Krassowsky1940,
		WGS84,
		WGS72,
		GRS67
	};

	Ellipsoid();
	Ellipsoid(Name name);

	double radius() const {return _radius;}
	double flattening() const {return _flattening;}

private:
	double _radius;
	double _flattening;
};

#endif // ELLIPSOID_H
