#ifndef LBLFILE_H
#define LBLFILE_H

#include <QImage>
#include "common/textcodec.h"
#include "subfile.h"
#include "label.h"

class HuffmanText;
class RGNFile;

class LBLFile : public SubFile
{
public:
	LBLFile(IMG *img)
	  : SubFile(img), _huffmanText(0), _table(0), _offset(0), _size(0),
	  _poiOffset(0), _poiSize(0), _imgOffsetIdSize(0), _poiMultiplier(0),
	  _multiplier(0), _encoding(0) {}
	LBLFile(const QString *path)
	  : SubFile(path), _huffmanText(0), _table(0), _offset(0), _size(0),
	  _poiOffset(0), _poiSize(0), _imgOffsetIdSize(0), _poiMultiplier(0),
	  _multiplier(0), _encoding(0) {}
	LBLFile(SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _huffmanText(0), _table(0), _offset(0), _size(0), _poiOffset(0),
	  _poiSize(0), _imgOffsetIdSize(0), _poiMultiplier(0), _multiplier(0),
	  _encoding(0) {}
	~LBLFile();

	bool load(Handle &hdl, const RGNFile *rgn, Handle &rgnHdl);
	void clear();

	Label label(Handle &hdl, quint32 offset, bool poi = false,
	  bool capitalize = true) const;

	quint8 imageIdSize() const {return _imgOffsetIdSize;}
	QImage readImage(Handle &hdl, quint32 id) const;

private:
	struct File {
		File() : offset(0), size(0) {}
		File(quint32 offset, quint32 size) : offset(offset), size(size) {}

		quint32 offset;
		quint32 size;
	};

	Label str2label(const QVector<quint8> &str, bool capitalize) const;
	Label label6b(Handle &hdl, quint32 offset, bool capitalize) const;
	Label label8b(Handle &hdl, quint32 offset, bool capitalize) const;
	Label labelHuffman(Handle &hdl, quint32 offset, bool capitalize) const;
	bool loadFiles(Handle &hdl, quint32 count, quint32 offset,
	  quint32 recordSize);

	HuffmanText *_huffmanText;
	quint32 *_table;
	TextCodec _codec;
	quint32 _offset;
	quint32 _size;
	quint32 _poiOffset;
	quint32 _poiSize;
	quint32 _imgOffset;
	quint32 _imgSize;
	quint8 _imgOffsetIdSize;
	quint8 _poiMultiplier;
	quint8 _multiplier;
	quint8 _encoding;

	QVector<File> _rasters;
};

#endif // LBLFILE_H
