#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QString>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(Q_OS_ANDROID)
#include <QTextCodec>
#else // QT 6 || ANDROID
#include <QStringDecoder>
#endif // QT 6 || ANDROID

class TextCodec
{
public:
	TextCodec();
	TextCodec(int codepage);

	QString toString(const QByteArray &ba);

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || defined(Q_OS_ANDROID)
	QTextCodec *_codec;
#else // QT 6.5
	QStringDecoder _decoder;
#endif // QT 6.5
};

#endif // TEXTCODEC_H
