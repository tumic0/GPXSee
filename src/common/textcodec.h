#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <QString>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) \
  || defined(Q_OS_ANDROID) || defined(Q_OS_MACOS)
#include <QTextCodec>
#else // QT 5 || ANDROID || MAC
#include <QStringDecoder>
#endif // QT 5 || ANDROID || MAC

class TextCodec
{
public:
	TextCodec();
	TextCodec(int codepage);

	QString toString(const QByteArray &ba);

private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) \
  || defined(Q_OS_ANDROID) || defined(Q_OS_MACOS)
	QTextCodec *_codec;
#else // QT 5 || ANDROID || MAC
	QStringDecoder _decoder;
#endif // QT 5 || ANDROID || MAC
};

#endif // TEXTCODEC_H
