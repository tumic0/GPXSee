#ifndef IMG_JLS_H
#define IMG_JLS_H

#include <QVector>
#include "map/matrix.h"
#include "subfile.h"

namespace IMG {

class JLS
{
public:
	JLS(quint16 maxval, quint16 near);

	bool decode(const SubFile *file, SubFile::Handle &hdl,
	  Matrix<qint16> &img) const;

private:
	class BitStream
	{
	public:
		BitStream(const SubFile *file, SubFile::Handle &hdl)
		  : _file(file), _hdl(hdl) {}

		bool init()
		{
			if (!_file->readVUInt32SW(_hdl, 4, _value))
				return false;
			_shift = -8;
			return true;
		}

		bool read(quint8 bits)
		{
			quint8 data;

			_value <<= bits;
			_shift += bits;

			while (_shift >= 0) {
				if (!_file->readByte(_hdl, &data))
					return false;

				_value |= (quint32)data << _shift;
				_shift -= 8;
			}

			return true;
		}

		quint32 value() const {return _value;}

	private:
		const SubFile *_file;
		SubFile::Handle &_hdl;
		quint32 _value;
		qint8 _shift;
	};

	struct Context
	{
		Context(quint16 width, quint16 range)
		{
			w = width;
			data = QVector<quint16>((w + 3) * 2);
			last = data.data();
			current = data.data() + (w + 3);

			runIndex = 0;
			rk = 0;
			rg = 1;
			lrk = 0;

			quint16 A = qMax(2, (range + 32) / 64);
			for (int i = 0; i < 4; i++) {
				a[i] = A;
				b[i] = 0;
				n[i] = 1;
			}
		}

		quint8 runIndex;
		quint8 rk;
		quint16 rg;
		quint16 n[4];
		quint16 a[4];
		qint16 b[4];
		quint8 lrk;

		quint16 w;
		QVector<quint16> data;
		quint16 *current;
		quint16 *last;
	};

	bool readLine(BitStream &bs, Context &ctx) const;
	bool processRunMode(BitStream &bs, Context &ctx, quint16 col,
	  quint16 &samples) const;
	bool decodeError(BitStream &bs, quint8 limit, quint8 k,
	  uint &MErrval) const;

	quint16 _maxval;
	quint16 _near;
	quint16 _range;
	quint8 _qbpp;
	quint8 _limit;
};

}

#endif // IMG_JLS_H
