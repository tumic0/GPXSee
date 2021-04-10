#ifndef MAPSFORGE_CMP_H
#define MAPSFORGE_CMP_H

#include <QByteArray>
#include <cstring>

namespace Mapsforge {

inline bool cmp(const QByteArray &b1, const QByteArray &b2)
{
	int len = b1.length();

	if (!len)
		return true;
	if (len != b2.length())
		return false;
	return !memcmp(b1.constData(), b2.constData(), len);
}

}

#endif // MAPSFORGE_CMP_H
