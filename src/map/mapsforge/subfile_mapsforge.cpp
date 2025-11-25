#include <cstring>
#include "subfile.h"

using namespace Mapsforge;

#define mod2n(x, m) ((x) & ((m) - 1));

bool SubFile::seek(quint64 pos)
{
	Q_ASSERT(pos < _size);

	int blockNum = pos >> BLOCK_BITS;

	if (_blockNum != blockNum) {
		quint64 seek = ((quint64)blockNum << BLOCK_BITS) + _offset;

		if (seek >= _offset + _size || !_file.seek(seek))
			return false;
		if (_file.read((char*)_data, sizeof(_data)) < 0)
			return false;
		_blockNum = blockNum;
	}

	_blockPos = mod2n(pos, 1U<<BLOCK_BITS);
	_pos = pos;

	return true;
}

bool SubFile::read(char *buff, quint32 size)
{
	while (size) {
		quint32 remaining = sizeof(_data) - _blockPos;
		if (size < remaining) {
			memcpy(buff, _data + _blockPos, size);
			_blockPos += size;
			_pos += size;
			return true;
		} else {
			memcpy(buff, _data + _blockPos, remaining);
			buff += remaining;
			size -= remaining;
			_blockPos = 0;
			_pos += remaining;
			if (!seek(_pos))
				return false;
		}
	}

	return true;
}
