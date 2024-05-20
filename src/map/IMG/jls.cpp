#include <cmath>
#include "jls.h"

using namespace IMG;

#define max(a, b) ((a) > (b) ? (a) : (b))

static const quint8 Z[] = {
	8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const int J[] = {
	0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
	4,  4,  5,  5,  6,  6,  7,  7,  8,  9, 10, 11, 12, 13, 14, 15
};

JLS::JLS(quint16 diff, quint16 factor)
{
	_maxval = diff;
	_near = factor;

	_range = ((_maxval + _near * 2) / (_near * 2 + 1)) + 1;
	_qbpp = ceil(log2(_range));
	quint8 bpp = max(2, ceil(log2(_maxval + 1)));
	quint8 LIMIT = 2 * (bpp + max(8, bpp));
	_limit = LIMIT - _qbpp - 1;
}

bool JLS::processRunMode(BitStream &bs, quint16 col, quint16 &samples)
{
	quint8 z;
	quint16 cnt = 0;

	while (true) {
		if ((qint32)bs.value() < 0) {
			z = Z[(bs.value() >> 0x18) ^ 0xff];

			for (quint8 i = 0; i < z; i++) {
				cnt = cnt + _rg;

				if (cnt <= col && _runIndex < 31) {
					_runIndex++;
					_rk = J[_runIndex];
					_rg = 1U << _rk;
				}

				if (cnt >= col) {
					if (!bs.read(i + 1))
						return 3;

					samples = col;
					return true;
				}
			}
		} else
			z = 0;

		if (z != 8) {
			if (!bs.read(z + 1))
				return false;

			if (_rk) {
				samples = (bs.value() >> (32 - _rk)) + cnt;
				if (!bs.read(_rk))
					return false;
			} else
				samples = cnt;

			_lrk = _rk + 1;
			if (_runIndex != 0) {
				_runIndex--;
				_rk = J[_runIndex];
				_rg = 1U << _rk;
			}

			return true;
		}

		if (!bs.read(8))
			return false;
	}
}

bool JLS::decodeError(BitStream &bs, quint8 limit, quint8 k, uint &MErrval)
{
	quint8 cnt = 0;
	MErrval = 0;

	while ((int)bs.value() >= 0) {
		cnt = Z[bs.value() >> 0x18];
		MErrval += cnt;
		if (bs.value() >> 0x18 != 0)
			break;

		if (!bs.read(8))
			return false;

		cnt = 0;
	}

	if (!bs.read(cnt + 1))
		return false;

	if (MErrval < limit) {
		if (k != 0) {
			MErrval = (bs.value() >> (0x20 - k)) + (MErrval << k);
			if (!bs.read(k))
				return false;
		}
	} else {
		MErrval = (bs.value() >> (0x20 - _qbpp)) + 1;
		if (!bs.read(_qbpp))
			return false;
	}

	return true;
}

bool JLS::readLine(BitStream &bs)
{
	quint8 ictx, rctx;
	quint8 k;
	uint MErrval;
	int Errval;
	int Rx = _last[1];
	int last0 = _last[0];
	int last1 = _last[1];
	uint col = 1;

	*_current = _last[1];

	do {
		if (abs(last1 - Rx) > _near) {
			int Px = Rx + last1 - last0;
			if (Px < 0)
				Px = 0;
			else if (Px > _maxval)
				Px = _maxval;

			for (k = 0; _n[1] << k < _a[1]; k++)
				;

			if (!decodeError(bs, _limit, k, MErrval))
				return false;

			int mes, meh;
			if (MErrval & 1) {
				meh = (MErrval + 1) >> 1;
				mes = -meh;
			} else {
				meh = MErrval >> 1;
				mes = meh;
			}
			if ((_near == 0) && (k == 0) && (_b[1] * 2 <= -_n[1])) {
				meh = mes + 1;
				mes = -mes - 1;
				if (MErrval & 1)
					meh = mes;
			} else
				mes = mes * (_near * 2 + 1);

			Errval = (Rx < last1) ? mes : -mes;
			Rx = Px + Errval;

			if (Rx < -_near)
				Rx += (_near * 2 + 1) * _range;
			else if (Rx > _maxval + _near)
				Rx -= (_near * 2 + 1) * _range;

			if (Rx < 0)
				Rx = 0;
			if (Rx > _maxval)
				Rx = _maxval;

			_a[1] = _a[1] + meh;
			_b[1] = _b[1] + mes;
			if (_n[1] == 0x40) {
				_a[1] = _a[1] >> 1;
				_b[1] = _b[1] >> 1;
				_n[1] = 0x21;
			} else {
				_n[1] = _n[1] + 1;
			}

			if (_b[1] <= -_n[1]) {
				_b[1] = _b[1] + _n[1];
				if (_b[1] <= -_n[1])
					_b[1] = 1 - _n[1];
			} else if (_b[1] > 0)
				_b[1] = ((_b[1] - _n[1]) >> 0xf) & (_b[1] - _n[1]);

			last0 = last1;
			last1 = _last[col + 1];
		} else {
			quint16 samples;
			if (!processRunMode(bs, _w - col + 1, samples))
				return false;

			if (samples != 0) {
				for (int i = 0; i < samples; i++) {
					if (col > _w)
						return false;
					_current[col] = Rx;
					col++;
				}

				if (col > _w)
					break;

				last0 = _last[col];
				last1 = _last[col + 1];
			} else {
				last0 = last1;
				last1 = _last[col + 1];
			}

			rctx = (abs(last0 - Rx) <= _near);
			quint16 TEMP = _a[rctx + 2];
			if (rctx)
				TEMP += _n[rctx + 2] >> 1;
			ictx = rctx | 2;

			for (k = 0; _n[rctx + 2] << k < TEMP; k++)
				;

			if (!decodeError(bs, _limit - _lrk, k, MErrval))
				return false;

			quint16 s = ((k == 0) && (rctx || MErrval)) ?
			  (_b[ictx] * 2 < _n[ictx]) : 0;

			Errval = MErrval + rctx + s;
			int evh;
			if ((Errval & 1) == 0) {
				Errval = Errval / 2;
				evh = Errval;
			} else {
				Errval = s - ((Errval + 1) >> 1);
				evh = -Errval;
				_b[ictx] = _b[ictx] + 1;
			}

			Errval *= (_near * 2 + 1);
			if (!rctx) {
				if (Rx == last0)
					return false;

				if (Rx < last0)
					Rx = last0 + Errval;
				else
					Rx = last0 - Errval;
			} else
				Rx = Rx + Errval;

			if (Rx < -_near)
				Rx += (_near * 2 + 1) * _range;
			else if (Rx > _maxval + _near)
				Rx -= (_near * 2 + 1) * _range;

			if (Rx < 0)
				Rx = 0;
			if (Rx > _maxval)
				Rx = _maxval;

			_a[ictx] = _a[ictx] + (evh - rctx);
			if (_n[ictx] == 0x40) {
				_a[ictx] = _a[ictx] >> 1;
				_b[ictx] = _b[ictx] >> 1;
				_n[ictx] = 0x21;
			} else
				_n[ictx] = _n[ictx] + 1;
		}

		_current[col] = Rx;
		col = col + 1;
	} while (col <= _w);

	return true;
}

bool JLS::decode(const SubFile *file, SubFile::Handle &hdl, Matrix<qint16> &img)
{
	BitStream bs(file, hdl);
	if (!bs.init())
		return false;

	_w = img.w();
	_data = QVector<quint16>((_w + 3) * 2);
	_last = _data.data();
	_current = _data.data() + (_w + 3);

	_runIndex = 0;
	_rk = 0;
	_rg = 1;
	_lrk = 0;

	quint16 A = max(2, (_range + 32) / 64);
	for (int i = 0; i < 4; i++) {
		_a[i] = A;
		_b[i] = 0;
		_n[i] = 1;
	}

	for (int i = 0; i < img.h(); i++) {
		if (!readLine(bs))
			return false;

		memcpy(&img.at(i, 0), _current + 1, _w * sizeof(quint16));

		quint16 *tmp = _last;
		_last = _current;
		_current = tmp;
	}

	return true;
}
