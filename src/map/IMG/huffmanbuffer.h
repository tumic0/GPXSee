#ifndef IMG_HUFFMANBUFFER_H
#define IMG_HUFFMANBUFFER_H

#include <QByteArray>
#include "subfile.h"

namespace IMG {

class RGNFile;

class HuffmanBuffer : public QByteArray
{
public:
	HuffmanBuffer(quint8 id) : _id(id) {}

	quint8 id() const {return _id;}
	bool load(const RGNFile *rgn, SubFile::Handle &rgnHdl);

private:
	quint8 _id;
};

}

#endif // IMG_HUFFMANBUFFER_H
