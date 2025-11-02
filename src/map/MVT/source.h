#ifndef SOURCE_H
#define SOURCE_H

#include <QByteArray>
#include "common/util.h"

namespace MVT {

class Source
{
public:
	Source() : _gzip(false), _mvt(false) {}
	Source(const QByteArray &data, bool gzip, bool mvt)
	  : _data(data), _gzip(gzip), _mvt(mvt) {}

	const QByteArray &data()
	{
		if (_gzip) {
			_data = Util::gunzip(_data);
			_gzip = false;
		}
		return _data;
	}
	bool mvt() const {return _mvt;}

private:
	QByteArray _data;
	bool _gzip;
	bool _mvt;
};

}

#endif // SOURCE_H
