#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QString>
#include <QMap>
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#include <QTextCodec>
#else // QT 6.5
#include <QStringDecoder>
#endif // QT 6.5

class TextCodec
{
public:
	TextCodec();
	TextCodec(int codepage);

	QString toString(const QByteArray &ba);

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
	QTextCodec *_codec;
#else // QT 6.5
	QStringDecoder _decoder;
#endif // QT 6.5
};

#endif // TEXTCODEC_H
