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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QTextCodec *_codec;
#else // QT6
	QString from8bCp(const QByteArray &ba) const;
	const char32_t *_table;
#endif // QT6
};

#endif // TEXTCODEC_H
