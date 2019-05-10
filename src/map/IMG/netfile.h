#ifndef NETFILE_H
#define NETFILE_H

#include "subfile.h"

class NETFile : public SubFile
{
public:
	NETFile(IMG *img, quint32 size) : SubFile(img, size), _init(false) {}

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset);

private:
	bool init();

	quint32 _offset;
	quint32 _size;
	quint8 _multiplier;

	bool _init;
};

#endif // NETFILE_H
