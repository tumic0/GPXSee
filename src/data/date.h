#ifndef DATE_H
#define DATE_H

#include <QtGlobal>

namespace Date
{
	inline qint64 delphi2unixMS(double date)
	{
		return (qint64)((date - 25569.0) * 86400000);
	}
}

#endif // DATE_H
