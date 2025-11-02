#include "common/protobuf.h"
#include "pbf.h"

using namespace Protobuf;
using namespace MVT;

static bool value(CTX &ctx, QVariant &val)
{
	QByteArray ba;
	quint64 num;
	double dnum;
	float fnum;
	quint32 len;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (field(ctx.tag)) {
			case 1:
				if (!str(ctx, ba))
					return false;
				val = QVariant(ba);
				break;
			case 2:
				if (!flt(ctx, fnum))
					return false;
				val = QVariant(fnum);
				break;
			case 3:
				if (!dbl(ctx, dnum))
					return false;
				val = QVariant(dnum);
				break;
			case 4:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(static_cast<qint64>(num));
				break;
			case 5:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(num);
				break;
			case 6:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(zigzag64decode(num));
				break;
			case 7:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(num ? true : false);
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ee);
}

static bool feature(CTX &ctx, PBF::Feature &f)
{
	quint32 len;
	quint32 e;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (field(ctx.tag)) {
			case 1:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, f.id))
					return false;
				break;
			case 2:
				if (!packed(ctx, f.tags))
					return false;
				break;
			case 3:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, e))
					return false;
				if (e > PBF::GeomType::POLYGON)
					return false;
				f.type = static_cast<PBF::GeomType>(e);
				break;
			case 4:
				if (!packed(ctx, f.geometry))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ee);
}

static bool layer(CTX &ctx, PBF::Layer &l)
{
	quint32 len;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (field(ctx.tag)) {
			case 1:
				if (!str(ctx, l.name))
					return false;
				break;
			case 2:
				l.features.append(PBF::Feature());
				if (!feature(ctx, l.features.last()))
					return false;
				break;
			case 3:
				l.keys.append(QByteArray());
				if (!str(ctx, l.keys.last()))
					return false;
				break;
			case 4:
				l.values.append(QVariant());
				if (!value(ctx, l.values.last()))
					return false;
				break;
			case 5:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, l.extent))
					return false;
				break;
			case 15:
				if (type(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, l.version))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ee);
}

static bool tile(const QByteArray &ba, QVector<PBF::Layer> &layers)
{
	CTX ctx(ba);

	while (ctx.bp < ctx.be) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (field(ctx.tag)) {
			case 3:
				layers.append(PBF::Layer());
				if (!layer(ctx, layers.last()))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ctx.be);
}

bool PBF::load(const QByteArray &ba)
{
	if (!tile(ba, _layers)) {
		_layers.clear();
		return false;
	} else
		return true;
}

void PBF::clear()
{
	_layers.clear();
}
