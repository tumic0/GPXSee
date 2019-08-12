#ifndef NETFILE_H
#define NETFILE_H

#include "subfile.h"

class NETFile : public SubFile
{
public:
	NETFile(IMG *img, quint32 size)
	  : SubFile(img, size), _offset(0), _size(0), _multiplier(0) {}

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset);

private:
	bool init();

	quint32 _offset;
	quint32 _size;
	quint8 _multiplier;
};

#endif // NETFILE_H
