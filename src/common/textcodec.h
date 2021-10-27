#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QString>

class QTextCodec;

class TextCodec
{
public:
	TextCodec();
	TextCodec(int codepage);

	QString toString(const QByteArray &ba) const;

private:
	QTextCodec *_codec;
};

#endif // TEXTCODEC_H
