#ifndef IMG_JLS_H
#define IMG_JLS_H

#include <QVector>
#include "bitstream.h"
#include "map/matrix.h"

namespace IMG {

class JLS
{
public:
	JLS(quint16 diff, quint16 factor, quint16 cols);

	bool decode(const SubFile *file, SubFile::Handle &hdl, Matrix<qint16> &img);

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
			_shift = (quint8)-8;
			return true;
		}

		bool read(quint8 bits)
		{
			quint8 data;

			_value <<= bits;
			_shift += bits;

			while (-1 < (char)_shift) {
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
		quint8 _shift;
	};

	bool readLine(BitStream &bs);
	bool processRunMode(BitStream &bs, quint16 col, quint16 &samples);
	bool decodeError(BitStream &bs, quint8 limit, quint8 k, uint &MErrval);

	quint16 _w;
	quint16 _maxval;
	quint16 _near;

	quint16 _range;
	quint8 _qbpp;
	quint8 _limit;

	quint8 _runIndex;
	quint8 _rk;
	quint16 _rg;
	quint16 _n[4];
	quint16 _a[4];
	qint16 _b[4];
	quint8 _lrk;

	QVector<quint16> _data;
	quint16 *_current;
	quint16 *_last;
};

}

#endif // IMG_JLS_H
