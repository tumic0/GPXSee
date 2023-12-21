#include <QTextCodec>
#include "textcodec.h"

/* When Qt is compiled with ICU support, QTextCodec::codecForName() is very
   slow due to broken codec name caching (the function does dozens of
   comparisons and only then asks the cache...), so we use our own map. */
static QMap<int, QTextCodec *> initCodecs()
{
	QMap<int, QTextCodec *> map;

	map.insert(65001, 0);
	map.insert(874, QTextCodec::codecForName("Windows-874"));
	map.insert(932, QTextCodec::codecForName("Shift-JIS"));
	map.insert(936, QTextCodec::codecForName("GB18030"));
	map.insert(949, QTextCodec::codecForName("EUC-KR"));
	map.insert(950, QTextCodec::codecForName("Big5"));
	map.insert(1250, QTextCodec::codecForName("Windows-1250"));
	map.insert(1251, QTextCodec::codecForName("Windows-1251"));
	map.insert(1252, QTextCodec::codecForName("Windows-1252"));
	map.insert(1253, QTextCodec::codecForName("Windows-1253"));
	map.insert(1254, QTextCodec::codecForName("Windows-1254"));
	map.insert(1255, QTextCodec::codecForName("Windows-1255"));
	map.insert(1256, QTextCodec::codecForName("Windows-1256"));
	map.insert(1257, QTextCodec::codecForName("Windows-1257"));
	map.insert(1258, QTextCodec::codecForName("Windows-1258"));

	return map;
}

const QMap<int, QTextCodec *> &TextCodec::codecs()
{
	static QMap<int, QTextCodec *> map = initCodecs();
	return map;
}

TextCodec::TextCodec(int codepage)
{
	const QMap<int, QTextCodec *> &map = codecs();

	QMap<int, QTextCodec *>::const_iterator it(map.find(codepage));
	if (it == map.cend()) {
		qWarning("%d: Unsupported codepage, using UTF-8", codepage);
		_codec = 0;
	} else
		_codec = *it;
}

QString TextCodec::toString(const QByteArray &ba) const
{
	return _codec ? _codec->toUnicode(ba) : QString::fromUtf8(ba);
}
