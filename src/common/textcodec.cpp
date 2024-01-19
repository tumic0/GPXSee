#include "textcodec.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)

static QTextCodec *codec(int mib)
{
	QTextCodec *c = QTextCodec::codecForMib(mib);
	if (!c)
		qWarning("MIB-%d: No such QTextCodec, using ISO-8859-1", mib);

	return c;
}

TextCodec::TextCodec() : _codec(0)
{
}

TextCodec::TextCodec(int codepage)
{
	switch (codepage) {
		case 874:
			_codec = codec(2109);
			break;
		case 932:
			_codec = codec(17);
			break;
		case 936:
			_codec = codec(114);
			break;
		case 949:
			_codec = codec(38);
			break;
		case 950:
			_codec = codec(2026);
			break;
		case 1250:
			_codec = codec(2250);
			break;
		case 1251:
			_codec = codec(2251);
			break;
		case 1252:
			_codec = codec(2252);
			break;
		case 1253:
			_codec = codec(2253);
			break;
		case 1254:
			_codec = codec(2254);
			break;
		case 1255:
			_codec = codec(2255);
			break;
		case 1256:
			_codec = codec(2256);
			break;
		case 1257:
			_codec = codec(2257);
			break;
		case 1258:
			_codec = codec(2258);
			break;
		case 65001:
			_codec = codec(106);
			break;
		default:
			qWarning("%d: Unknown codepage, using ISO-8859-1", codepage);
			_codec = 0;
	}
}

QString TextCodec::toString(const QByteArray &ba)
{
	return _codec ? _codec->toUnicode(ba) : QString::fromLatin1(ba);
}

#else // QT 6.5

TextCodec::TextCodec()
{
}

TextCodec::TextCodec(int codepage)
{
	if (codepage == 65001)
		_decoder = QStringDecoder(QStringDecoder::Utf8);
	else {
		QByteArray cp(QByteArray("CP") + QByteArray::number(codepage));
		_decoder = QStringDecoder(cp.constData());

		if (!_decoder.isValid())
			qWarning("%d: Unknown codepage, using ISO-8859-1", codepage);
	}
}

QString TextCodec::toString(const QByteArray &ba)
{
	return _decoder.isValid() ? _decoder.decode(ba) : QString::fromLatin1(ba);
}
#endif // QT 6.5
