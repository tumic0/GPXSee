#ifndef UTIL_H
#define UTIL_H

#include <QString>

namespace Util
{
	int str2int(const char *str, int len);
	double niceNum(double x, bool round);
	QString file2name(const QString &path);
}

#endif // UTIL_H
