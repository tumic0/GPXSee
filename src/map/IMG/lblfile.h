#ifndef LBLFILE_H
#define LBLFILE_H

#include "subfile.h"
#include "label.h"

class QTextCodec;

class LBLFile : public SubFile
{
public:
	LBLFile(IMG *img)
	  : SubFile(img), _codec(0), _offset(0), _size(0), _poiOffset(0),
	  _poiSize(0), _poiMultiplier(0), _multiplier(0), _encoding(0) {}
	LBLFile(const QString &path)
	  : SubFile(path), _codec(0), _offset(0), _size(0), _poiOffset(0),
	  _poiSize(0), _poiMultiplier(0), _multiplier(0), _encoding(0) {}
	LBLFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _codec(0), _offset(0), _size(0), _poiOffset(0), _poiSize(0),
	  _poiMultiplier(0), _multiplier(0), _encoding(0) {}

	Label label(Handle &hdl, quint32 offset, bool poi = false);

private:
	bool init(Handle &hdl);

	Label label6b(Handle &hdl, quint32 offset) const;
	Label label8b(Handle &hdl, quint32 offset) const;

	QTextCodec *_codec;
	quint32 _offset;
	quint32 _size;
	quint32 _poiOffset;
	quint32 _poiSize;
	quint8 _poiMultiplier;
	quint8 _multiplier;
	quint8 _encoding;
};

#endif // LBLFILE_H
