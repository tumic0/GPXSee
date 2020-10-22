#include <QTextCodec>
#include "lblfile.h"

enum Charset {Normal, Symbol, Special};

static quint8 NORMAL_CHARS[] = {
	' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z',  '~', '~', '~', ' ', ' ',
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

static bool isAllUpperCase(const QString &str)
{
	if (str.isEmpty())
		return false;
	for (int i = 0; i < str.size(); i++)
		if (str.at(i).isLetter() && !(str.at(i).isUpper()
		  || str.at(i) == QChar(0x00DF)))
			return false;

	return true;
}

static QString capitalized(const QString &str)
{
	QString ret(str);
	for (int i = 0; i < str.size(); i++)
		if (i && !str.at(i-1).isSpace())
			ret[i] = str.at(i).toLower();
		else
			ret[i] = str.at(i);

	return ret;
}


bool LBLFile::init(Handle &hdl)
{
	quint16 codepage;
	quint8 multiplier, poiMultiplier;

	if (!(seek(hdl, _gmpOffset + 0x15) && readUInt32(hdl, _offset)
	  && readUInt32(hdl, _size) && readUInt8(hdl, multiplier)
	  && readUInt8(hdl, _encoding) && seek(hdl, _gmpOffset + 0x57)
	  && readUInt32(hdl, _poiOffset) && readUInt32(hdl, _poiSize)
	  && readUInt8(hdl, poiMultiplier) && seek(hdl, _gmpOffset + 0xAA)
	  && readUInt16(hdl, codepage)))
		return false;

	_multiplier = 1<<multiplier;
	_poiMultiplier = 1<<poiMultiplier;

	if (codepage == 65001)
		_codec = QTextCodec::codecForName("UTF-8");
	else if (codepage == 0)
		_codec = 0;
	else
		_codec = QTextCodec::codecForName(QString("CP%1").arg(codepage)
		  .toLatin1());

	return true;
}

Label LBLFile::label6b(Handle &hdl, quint32 offset, bool capitalize) const
{
	Label::Shield::Type shieldType = Label::Shield::None;
	QByteArray label, shieldLabel;
	QByteArray *bap = &label;
	Charset curCharSet = Normal;
	quint8 b1, b2, b3;

	if (!seek(hdl, offset))
		return Label();

	while (true) {
		if (!(readUInt8(hdl, b1) && readUInt8(hdl, b2) && readUInt8(hdl, b3)))
			return Label();

		int c[]= {b1>>2, (b1&0x3)<<4|b2>>4, (b2&0xF)<<2|b3>>6, b3&0x3F};

		for (int cpt = 0; cpt < 4; cpt++) {
			if (c[cpt] > 0x2f || (curCharSet == Normal && c[cpt] == 0x1d)) {
				QString text(QString::fromLatin1(label));
				return Label(capitalize && isAllUpperCase(text)
				  ? capitalized(text) : text, Label::Shield(shieldType,
				  shieldLabel));
			}
			switch (curCharSet) {
				case Normal:
					if (c[cpt] == 0x1c)
						curCharSet = Symbol;
					else if (c[cpt] == 0x1b)
						curCharSet = Special;
					else if (c[cpt] >= 0x2a && c[cpt] <= 0x2f) {
						shieldType = static_cast<Label::Shield::Type>
						  (c[cpt] - 0x29);
						bap = &shieldLabel;
					} else if (bap == &shieldLabel
					  && NORMAL_CHARS[c[cpt]] == ' ')
						bap = &label;
					else
						bap->append(NORMAL_CHARS[c[cpt]]);
					break;
				case Symbol:
					bap->append(SYMBOL_CHARS[c[cpt]]);
					curCharSet = Normal;
					break;
				case Special:
					bap->append(SPECIAL_CHARS[c[cpt]]);
					curCharSet = Normal;
					break;
			}
		}
	}
}

Label LBLFile::label8b(Handle &hdl, quint32 offset, bool capitalize) const
{
	Label::Shield::Type shieldType = Label::Shield::None;
	QByteArray label, shieldLabel;
	QByteArray *bap = &label;
	quint8 c;

	if (!seek(hdl, offset))
		return Label();

	while (true) {
		if (!readUInt8(hdl, c))
			return Label();
		if (!c || c == 0x1d)
			break;

		if (c == 0x1c)
			capitalize = false;
		else if ((c >= 0x1e && c <= 0x1f)) {
			if (bap == &shieldLabel)
				bap = &label;
			else
				bap->append(' ');
		} else if (c <= 0x07) {
			shieldType = static_cast<Label::Shield::Type>(c);
			bap = &shieldLabel;
		} else if (bap == &shieldLabel && c == 0x20) {
			bap = &label;
		} else
			bap->append(c);
	}

	QString text(_codec ? _codec->toUnicode(label) : QString::fromLatin1(label));
	QString shieldText(_codec ? _codec->toUnicode(shieldLabel)
	  : QString::fromLatin1(shieldLabel));

	return Label(capitalize && isAllUpperCase(text) ? capitalized(text) : text,
	  Label::Shield(shieldType, shieldText));
}

Label LBLFile::label(Handle &hdl, quint32 offset, bool poi, bool capitalize)
{
	if (!_multiplier && !init(hdl))
		return QString();

	quint32 labelOffset;
	if (poi) {
		quint32 poiOffset;
		if (!(_poiSize >= offset * _poiMultiplier
		  && seek(hdl, _poiOffset + offset * _poiMultiplier)
		  && readUInt24(hdl, poiOffset) && (poiOffset & 0x3FFFFF)))
			return QString();
		labelOffset = _offset + (poiOffset & 0x3FFFFF) * _multiplier;
	} else
		labelOffset = _offset + offset * _multiplier;

	if (labelOffset > _offset + _size)
		return QString();

	switch (_encoding) {
		case 6:
			return label6b(hdl, labelOffset, capitalize);
		case 9:
		case 10:
			return label8b(hdl, labelOffset, capitalize);
		default:
			return Label();
	}
}
