#include <cctype>
#include <cmath>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QImageReader>
#ifdef Q_OS_ANDROID
#include <QUrl>
#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>
#endif // Q_OS_ANDROID
#include <zlib.h>
#include "util.h"

#define SQLITE_DB_MAGIC "SQLite format 3"

#ifdef Q_OS_ANDROID
static QString documentName(const QString &path)
{
	QJniEnvironment env;

	QJniObject urlString = QJniObject::fromString(path);
	QJniObject uri = QJniObject::callStaticObjectMethod("android/net/Uri",
	  "parse", "(Ljava/lang/String;)Landroid/net/Uri;",
	  urlString.object<jstring>());
	if (!uri.isValid()) {
		env->ExceptionClear();
		return QString();
	}
	QJniObject context = QNativeInterface::QAndroidApplication::context();
	if (!context.isValid()) {
		env->ExceptionClear();
		return QString();
	}
	QJniObject contentResolver = context.callObjectMethod(
	  "getContentResolver", "()Landroid/content/ContentResolver;");
	if (!contentResolver.isValid()) {
		env->ExceptionClear();
		return QString();
	}
	QJniObject columnName = QJniObject::getStaticObjectField<jstring>(
	  "android/provider/MediaStore$MediaColumns", "DISPLAY_NAME");
	if (!columnName.isValid()) {
		env->ExceptionClear();
		return QString();
	}
	jobjectArray stringArray = env->NewObjectArray(
	  1, env->FindClass("java/lang/String"), 0);
	env->SetObjectArrayElement(stringArray, 0, columnName.object<jstring>());
	QJniObject cursor = contentResolver.callObjectMethod("query",
	  "(Landroid/net/Uri;[Ljava/lang/String;Landroid/os/Bundle;"
	  "Landroid/os/CancellationSignal;)Landroid/database/Cursor;",
	  uri.object(), stringArray, 0, 0);
	if (!cursor.isValid()) {
		env->ExceptionClear();
		return QString();
	}
	if (!cursor.callMethod<jboolean>("moveToFirst")) {
		env->ExceptionClear();
		return QString();
	}
	QJniObject str = cursor.callObjectMethod("getString",
	  "(I)Ljava/lang/String;", 0);
	if (!str.isValid()) {
		env->ExceptionClear();
		return QString();
	}

	return str.toString();
}
#endif // Q_OS_ANDROID

int Util::str2int(const char *str, int len)
{
	int res = 0;

	for (const char *sp = str; sp < str + len; sp++) {
		if (::isdigit(*sp))
			res = res * 10 + *sp - '0';
		else
			return -1;
	}

	return res;
}

double Util::niceNum(double x, bool round)
{
	int expv;
	double f;
	double nf;

	expv = (int)floor(log10(x));
	f = x / pow(10.0, expv);

	if (round) {
		if (f < 1.5)
			nf = 1.0;
		else if (f < 3.0)
			nf = 2.0;
		else if (f < 7.0)
			nf = 5.0;
		else
			nf = 10.0;
	} else {
		if (f <= 1.0)
			nf = 1.0;
		else if (f <= 2.0)
			nf = 2.0;
		else if (f <= 5.0)
			nf = 5.0;
		else
			nf = 10.0;
	}

	return nf * pow(10.0, expv);
}

int Util::log2i(unsigned val)
{
	int ret = 0;

	while (val >>= 1)
		ret++;

	return ret;
}

QString Util::file2name(const QString &path)
{
	QFileInfo fi(displayName(path));
	return fi.baseName().replace('_', ' ');
}

QString Util::displayName(const QString &path)
{
#ifdef Q_OS_ANDROID
	QUrl url(path);

	// Not an Android URL, return standard filename.
	if (url.scheme() != "content")
		return QFileInfo(path).fileName();
	// Directory browsing URLs. Those can not be translated using the Android
	// content resolver but we can get the filename from the URL path.
	else if (url.path().startsWith("/tree/"))
		return QFileInfo(url.fileName()).fileName();
	// Translate all "regular" android URLs using the Android content resolver.
	else
		return documentName(path);
#else
	return path;
#endif // Q_OS_ANDROID
}

const QTemporaryDir &Util::tempDir()
{
	static QTemporaryDir dir;
	return dir;
}

bool Util::isSQLiteDB(const QString &path, QString &errorString)
{
	QFile file(path);
	char magic[sizeof(SQLITE_DB_MAGIC)];

	if (!file.open(QFile::ReadOnly)) {
		errorString = file.errorString();
		return false;
	}
	if ((file.read(magic, sizeof(magic)) < (qint64)sizeof(magic))
	  || memcmp(SQLITE_DB_MAGIC, magic, sizeof(SQLITE_DB_MAGIC))) {
		errorString = "Not a SQLite database file";
		return false;
	}

	return true;
}

QImage Util::svg2img(const QString &path, qreal ratio)
{
	QImageReader ir(path, "svg");
	QSize s(ir.size());
	if (!s.isValid())
		return QImage();

	ir.setScaledSize(QSize(s.width() * ratio, s.height() * ratio));
	QImage img(ir.read());
	img.setDevicePixelRatio(ratio);

	return img;
}

QByteArray Util::gunzip(const QByteArray &data)
{
	quint32 size = 0;
	const quint8 *isp = (const quint8*)data.constData() + data.size()
	  - sizeof(size);
	for (quint32 i = 0; i < sizeof(size); i++)
		size |= ((quint32)*(isp + i)) << (i * 8);

	QByteArray uba;
	uba.resize(size);

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = data.size();
	strm.next_in = (Bytef*)data.constData();
	strm.avail_out = uba.size();
	strm.next_out = (Bytef*)uba.data();

	if (inflateInit2(&strm, MAX_WBITS + 16) != Z_OK)
		return QByteArray();
	int ret = inflate(&strm, Z_NO_FLUSH);
	(void)inflateEnd(&strm);

	return (ret == Z_STREAM_END) ? uba : QByteArray();
}
