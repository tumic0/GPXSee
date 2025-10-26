#ifndef PROTOBUF_H
#define PROTOBUF_H

#include <QByteArray>

namespace Protobuf
{
	constexpr quint32 VARINT = 0;
	constexpr quint32 I64    = 1;
	constexpr quint32 LEN    = 2;
	constexpr quint32 I32    = 5;

	struct CTX
	{
		CTX(const QByteArray &ba)
		  : bp(ba.constData()), be(bp + ba.size()), tag(0) {}

		const char *bp;
		const char *be;
		quint32 tag;
	};

	inline qint32 zigzag32decode(quint32 value)
	{
		return static_cast<qint32>((value >> 1u) ^ static_cast<quint32>(
		  -static_cast<qint32>(value & 1u)));
	}

	inline qint64 zigzag64decode(quint64 value)
	{
		return static_cast<qint64>((value >> 1u) ^ static_cast<quint64>(
		  -static_cast<qint64>(value & 1u)));
	}

	template<typename T>
	inline bool varint(CTX &ctx, T &val)
	{
		unsigned int shift = 0;
		val = 0;

		while ((ctx.bp < ctx.be) && (shift < sizeof(T) * 8)) {
			val |= static_cast<T>((quint8)*ctx.bp & 0x7F) << shift;
			shift += 7;
			if (!((quint8)*ctx.bp++ & 0x80))
				return true;
		}

		return false;
	}

	inline quint32 type(quint32 tag) {return (tag & 0x07);}
	inline quint32 field(quint32 tag) {return (tag >> 3);}

	bool length(CTX &ctx, quint32 &val);
	bool str(CTX &ctx, QByteArray &val);
	bool dbl(CTX &ctx, double &val);
	bool flt(CTX &ctx, float &val);
	bool packed(CTX &ctx, QVector<quint32> &vals);
	bool skip(CTX &ctx);
}

#endif // PROTOBUF_H
