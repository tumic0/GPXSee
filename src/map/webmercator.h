#ifndef WEBMERCATOR_H
#define WEBMERCATOR_H

#include "ct.h"

class WebMercator : public CT
{
public:
	virtual CT *clone() const {return new WebMercator(*this);}
	virtual bool operator==(const CT &ct) const;

	virtual PointD ll2xy(const Coordinates &c) const;
	virtual Coordinates xy2ll(const PointD &p) const;
};

#endif // WEBMERCATOR_H
