#include "rgnfile.h"
#include "huffmanbuffer.h"

bool HuffmanBuffer::load(const RGNFile *rgn, SubFile::Handle &rgnHdl)
{
	quint32 recordSize, recordOffset = rgn->dictOffset();

	for (int i = 0; i <= _id; i++) {
		if (!rgn->seek(rgnHdl, recordOffset))
			return false;
		if (!rgn->readVUInt32(rgnHdl, recordSize))
			return false;
		recordOffset = rgn->pos(rgnHdl) + recordSize;
		if (recordOffset > rgn->dictOffset() + rgn->dictSize())
			return false;
	};

	resize(recordSize);
	for (int i = 0; i < QByteArray::size(); i++)
		if (!rgn->readUInt8(rgnHdl, *((quint8*)(data() + i))))
			return false;

	return true;
}
