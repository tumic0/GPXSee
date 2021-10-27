#include <QTextCodec>
#include "textcodec.h"

TextCodec::TextCodec()
{
	_codec = QTextCodec::codecForName("Windows-1252");
}

TextCodec::TextCodec(int codepage)
{
	switch (codepage) {
		case 65001:
			_codec = 0;
			break;
		case 932:
			_codec = QTextCodec::codecForName("Shift-JIS");
			break;
		case 936:
			_codec = QTextCodec::codecForName("GB18030");
			break;
		case 949:
			_codec = QTextCodec::codecForName("EUC-KR");
			break;
		case 950:
			_codec = QTextCodec::codecForName("Big5");
			break;
		case 1250:
			_codec = QTextCodec::codecForName("Windows-1250");
			break;
		case 1251:
			_codec = QTextCodec::codecForName("Windows-1251");
			break;
		case 1253:
			_codec = QTextCodec::codecForName("Windows-1253");
			break;
		case 1254:
			_codec = QTextCodec::codecForName("Windows-1254");
			break;
		case 1255:
			_codec = QTextCodec::codecForName("Windows-1255");
			break;
		case 1256:
			_codec = QTextCodec::codecForName("Windows-1256");
			break;
		case 1257:
			_codec = QTextCodec::codecForName("Windows-1257");
			break;
		case 1258:
			_codec = QTextCodec::codecForName("Windows-1258");
			break;
		default:
			_codec = QTextCodec::codecForName("Windows-1252");
	}
}

QString TextCodec::toString(const QByteArray &ba) const
{
	return _codec ? _codec->toUnicode(ba) : QString::fromUtf8(ba);
}
