#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QImage>

class QTemporaryDir;

#define ARRAY_SIZE(array) \
  (sizeof(array) / sizeof(array[0]))

namespace Util
{
	int log2i(unsigned val);
	int str2int(const char *str, int len);
	double niceNum(double x, bool round);
	QString file2name(const QString &path);
	QString displayName(const QString &path);
	const QTemporaryDir &tempDir();
	bool isSQLiteDB(const QString &path, QString &errorString);
	QImage svg2img(const QString &path, qreal ratio);
}

#endif // UTIL_H
