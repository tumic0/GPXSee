#ifndef NETFILE_H
#define NETFILE_H

#include "subfile.h"

class NETFile : public SubFile
{
public:
	NETFile(IMG *img)
	  : SubFile(img), _offset(0), _size(0), _multiplier(0) {}
	NETFile(const QString &path)
	  : SubFile(path), _offset(0), _size(0), _multiplier(0) {}
	NETFile(SubFile *gmp, quint32 offset)
	  : SubFile(gmp, offset), _offset(0), _size(0), _multiplier(0) {}

	bool lblOffset(Handle &hdl, quint32 netOffset, quint32 &lblOffset);

private:
	bool init(Handle &hdl);

	quint32 _offset, _size;
	quint8 _multiplier;
};

#endif // NETFILE_H
