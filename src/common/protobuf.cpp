#include <QVector>
#include "protobuf.h"

using namespace Protobuf;

bool Protobuf::length(CTX &ctx, quint32 &val)
{
	if (type(ctx.tag) != LEN)
		return false;

	if (!varint(ctx, val))
		return false;

	return true;
}

bool Protobuf::str(CTX &ctx, QByteArray &val)
{
	quint32 len;

	if (!length(ctx, len))
		return false;
	if (ctx.bp + len > ctx.be)
		return false;

/* In Qt5 the (later) conversion to QString is broken when the QByteArray is
   not nul terminated so we have to use the "deep copy" constructor that
   nul-terminates the byte array when it is created. */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	val = QByteArray(ctx.bp, len);
#else
	val = QByteArray::fromRawData(ctx.bp, len);
#endif
	ctx.bp += len;

	return true;
}

bool Protobuf::dbl(CTX &ctx, double &val)
{
	if (type(ctx.tag) != I64)
		return false;
	if (ctx.bp + sizeof(val) > ctx.be)
		return false;

	memcpy(&val, ctx.bp, sizeof(val));
	ctx.bp += sizeof(val);

	return true;
}

bool Protobuf::flt(CTX &ctx, float &val)
{
	if (type(ctx.tag) != I32)
		return false;
	if (ctx.bp + sizeof(val) > ctx.be)
		return false;

	memcpy(&val, ctx.bp, sizeof(val));
	ctx.bp += sizeof(val);

	return true;
}

bool Protobuf::packed(CTX &ctx, QVector<quint32> &vals)
{
	quint32 v;
	const char *end;

	switch (type(ctx.tag)) {
		case LEN:
			if (!varint(ctx, v))
				return false;
			end = ctx.bp + v;
			if (end > ctx.be)
				return false;
			while (ctx.bp < end) {
				if (!varint(ctx, v))
					return false;
				vals.append(v);
			}
			return (ctx.bp == end);
		case VARINT:
			if (!varint(ctx, v))
				return false;
			vals.append(v);
			return true;
		default:
			return false;
	}
}

bool Protobuf::skip(CTX &ctx)
{
	quint32 len = 0;

	switch (type(ctx.tag)) {
		case VARINT:
			return varint(ctx, len);
		case I64:
			len = 8;
			break;
		case LEN:
			if (!varint(ctx, len))
				return false;
			break;
		case I32:
			len = 4;
			break;
		default:
			return false;
	}

	if (ctx.bp + len > ctx.be)
		return false;
	ctx.bp += len;

	return true;
}
