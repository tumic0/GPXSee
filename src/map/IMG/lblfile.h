#ifndef LBLFILE_H
#define LBLFILE_H

#include "subfile.h"
#include "label.h"

class QTextCodec;

class LBLFile : public SubFile
{
public:
	LBLFile(IMG *img, quint32 size) : SubFile(img, size), _init(false) {}

	Label label(Handle &hdl, quint32 offset, bool poi = false);

private:
	bool init();

	Label label6b(Handle &hdl, quint32 offset) const;
	Label label8b(Handle &hdl, quint32 offset) const;

	quint32 _offset;
	quint32 _size;
	quint32 _poiOffset;
	quint32 _poiSize;
	quint8 _multiplier;
	quint8 _encoding;
	QTextCodec *_codec;

	bool _init;
};

#endif // LBLFILE_H
