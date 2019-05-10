#include <QTextCodec>
#include "lblfile.h"

enum Charset {Normal, Symbol, Special};

static quint8 NORMAL_CHARS[] = {
	' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z',  '~', '~', '~', '~', '~',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '~', '~', '~', '~', '~', '~'
};

static quint8 SYMBOL_CHARS[] = {
	'@', '!', '"', '#', '$', '%', '&', '\'',
	'(', ')', '*', '+', ',', '-', '.', '/',
	'~', '~', '~', '~', '~', '~', '~', '~',
	'~', '~', ':', ';', '<', '=', '>', '?',
	'~', '~', '~', '~', '~', '~', '~', '~',
	'~', '~', '~', '[', '\\', ']', '^', '_'
};

static quint8 SPECIAL_CHARS[] = {
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
	'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
	'x', 'y', 'z', '~', '~', '~', '~', '~',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '~', '~', '~', '~', '~', '~'
};

static QString capitalize(const QString &str)
{
	if (str.isEmpty())
		return str;
	for (int i = 0; i < str.size(); i++)
		if (str.at(i).isLetter() && !str.at(i).isUpper())
			return str;

	QString ret(str);
	for (int i = 0; i < str.size(); i++)
		if (i && !str.at(i-1).isSpace())
			ret[i] = str.at(i).toLower();
		else
			ret[i] = str.at(i);

	return ret;
}

bool LBLFile::init()
{
	Handle hdl;
	quint16 codepage;

	if (!(seek(hdl, 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size) && readByte(hdl, _multiplier)
	  && readByte(hdl, _encoding) && seek(hdl, 0x57)
	  && readUInt32(hdl, _poiOffset) && readUInt32(hdl, _poiSize)
	  && seek(hdl, 0xAA) && readUInt16(hdl, codepage)))
		return false;

	_multiplier = 1<<_multiplier;

	if (codepage == 65001)
		_codec = QTextCodec::codecForName("UTF-8");
	else if (codepage == 0)
		_codec = 0;
	else
		_codec = QTextCodec::codecForName(QString("CP%1").arg(codepage)
		  .toLatin1());

	return true;
}

QString LBLFile::label6b(Handle &hdl, quint32 offset) const
{
	QByteArray result;
	Charset curCharSet = Normal;
	quint8 b1, b2, b3;

	if (!seek(hdl, offset))
		return QString();

	while (true) {
		if (!(readByte(hdl, b1) && readByte(hdl, b2) && readByte(hdl, b3)))
			return QString();

		int c[]= {b1>>2, (b1&0x3)<<4|b2>>4, (b2&0xF)<<2|b3>>6, b3&0x3F};

		for (int cpt = 0; cpt < 4; cpt++) {
			if (c[cpt] > 0x2F)
				return QString::fromLatin1(result);
			switch (curCharSet) {
				case Normal:
					if (c[cpt] == 0x1c)
						curCharSet = Symbol;
					else if (c[cpt] == 0x1b)
						curCharSet = Special;
					else if(c[cpt] == 0x1d)
						result.append('|');
					else if (c[cpt] == 0x1f)
						result.append(' ');
					else if (c[cpt] == 0x1e)
						result.append(' ');
					else
						result.append(NORMAL_CHARS[c[cpt]]);
					break;
				case Symbol:
					result.append(SYMBOL_CHARS[c[cpt]]);
					curCharSet = Normal;
					break;
				case Special:
					result.append(SPECIAL_CHARS[c[cpt]]);
					curCharSet = Normal;
					break;
			}
		}
	}
}

QString LBLFile::label8b(Handle &hdl, quint32 offset) const
{
	QByteArray result;
	quint8 c;

	if (!seek(hdl, offset))
		return QString();

	while (true) {
		if (!readByte(hdl, c))
			return QString();
		if (!c)
			break;

		if (c >= 0x1B && c <= 0x1F)
			result.append(' ');
		else if (c > 0x07)
			result.append(c);
	}

	return _codec ? _codec->toUnicode(result) : QString::fromLatin1(result);
}

QString LBLFile::label(Handle &hdl, quint32 offset, bool poi)
{
	if (!_init) {
		if (!(_init = init()))
			return QString();
	}

	quint32 labelOffset;
	if (poi) {
		quint32 poiOffset;
		if (!(seek(hdl, _poiOffset + offset) && readUInt24(hdl, poiOffset)))
			return QString();
		labelOffset = _offset + (poiOffset & 0x3FFFFF) * _multiplier;
	} else
		labelOffset = _offset + offset * _multiplier;

	if (labelOffset > _offset + _size)
		return QString();

	switch (_encoding) {
		case 6:
			return capitalize(label6b(hdl, labelOffset));
		case 9:
		case 10:
			return capitalize(label8b(hdl, labelOffset));
		default:
			return QString();
	}
}
