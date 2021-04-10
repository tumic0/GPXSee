#include "rgnfile.h"
#include "huffmanbuffer.h"

using namespace IMG;

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
	return rgn->read(rgnHdl, data(), recordSize);
}
