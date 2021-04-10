#ifndef IMG_IMGDATA_H
#define IMG_IMGDATA_H

#include "mapdata.h"

class QFile;

namespace IMG {

class IMGData : public MapData
{
public:
	IMGData(const QString &fileName);

	const QString &fileName() const {return _fileName;}

	unsigned blockBits() const {return _blockBits;}
	bool readBlock(QFile &file, int blockNum, char *data) const;

private:
	qint64 read(QFile &file, char *data, qint64 maxSize) const;
	template<class T> bool readValue(QFile &file, T &val) const;

	QString _fileName;
	quint8 _key;
	unsigned _blockBits;
};

}

#endif // IMG_IMGDATA_H
