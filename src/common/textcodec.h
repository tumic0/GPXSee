#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QString>
#include <QMap>

class QTextCodec;

class TextCodec
{
public:
	TextCodec() : _codec(0) {}
	TextCodec(int codepage);

	QString toString(const QByteArray &ba) const;

private:
	QTextCodec *_codec;
};

#endif // TEXTCODEC_H
