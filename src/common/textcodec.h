#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QString>

class TextCodec
{
public:
	TextCodec();
	TextCodec(int codepage);

	QString toString(const QByteArray &ba) const;

private:
	QString from8bCp(const QByteArray &ba) const;

	const char32_t *_table;
};

#endif // TEXTCODEC_H
