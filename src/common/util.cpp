#include <cctype>
#include <cmath>
#include <QFileInfo>
#include <QTemporaryDir>
#ifdef Q_OS_ANDROID
#include <QUrl>
#include <QCoreApplication>
#include <QJniEnvironment>
#include <QJniObject>
#endif // Q_OS_ANDROID
#include "util.h"


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
	if (url.scheme() != "content") {
		QFileInfo fi(path);
		return fi.fileName();
	// Directory browsing URLs. Those can not be translated using the Android
	// content resolver but we can get the filename from the URL path.
	} else if (url.path().startsWith("/tree/")) {
		QFileInfo fi(url.fileName());
		return fi.fileName();
	// Translate all "regular" android URLs using the Android content resolver.
	} else
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
