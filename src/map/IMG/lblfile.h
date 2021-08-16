#ifndef IMG_LBLFILE_H
#define IMG_LBLFILE_H

#include <QPixmap>
#include "common/textcodec.h"
#include "subfile.h"
#include "label.h"

namespace IMG {

class HuffmanText;
class RGNFile;

class LBLFile : public SubFile
{
public:
	LBLFile(const IMGData *img)
	  : SubFile(img), _huffmanText(0), _table(0), _rasters(0), _offset(0),
	  _size(0), _poiOffset(0), _poiSize(0), _imgOffsetIdSize(0),
	  _poiMultiplier(0), _multiplier(0), _encoding(0) {}
	LBLFile(const QString *path)
	  : SubFile(path), _huffmanText(0), _table(0), _rasters(0), _offset(0),
	  _size(0), _poiOffset(0), _poiSize(0), _imgOffsetIdSize(0),
	  _poiMultiplier(0), _multiplier(0), _encoding(0) {}
	LBLFile(const SubFile *gmp, quint32 offset) : SubFile(gmp, offset),
	  _huffmanText(0), _table(0), _rasters(0), _offset(0), _size(0),
	  _poiOffset(0), _poiSize(0), _imgOffsetIdSize(0), _poiMultiplier(0),
	  _multiplier(0), _encoding(0) {}
	~LBLFile();

	bool load(Handle &hdl, const RGNFile *rgn, Handle &rgnHdl);
	void clear();

	Label label(Handle &hdl, quint32 offset, bool poi = false,
	  bool capitalize = true, bool convert = false) const;

	quint8 imageIdSize() const {return _imgOffsetIdSize;}
	QPixmap image(Handle &hdl, quint32 id) const;

private:
	struct Image {
		quint32 offset;
		quint32 size;
	};

	Label str2label(const QVector<quint8> &str, bool capitalize,
	  bool convert) const;
	Label label6b(Handle &hdl, quint32 offset, bool capitalize,
	  bool convert) const;
	Label label8b(Handle &hdl, quint32 offset, bool capitalize,
	  bool convert) const;
	Label labelHuffman(Handle &hdl, quint32 offset, bool capitalize,
	  bool convert) const;
	bool loadRasterTable(Handle &hdl, quint32 offset, quint32 size,
	  quint32 recordSize);

	HuffmanText *_huffmanText;
	quint32 *_table;
	Image *_rasters;
	TextCodec _codec;
	quint32 _offset;
	quint32 _size;
	quint32 _poiOffset;
	quint32 _poiSize;
	quint32 _imgOffset;
	quint32 _imgSize;
	quint32 _imgCount;
	quint8 _imgOffsetIdSize;
	quint8 _poiMultiplier;
	quint8 _multiplier;
	quint8 _encoding;
};

}

#endif // IMG_LBLFILE_H
