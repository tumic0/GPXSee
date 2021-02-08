#ifndef IMG_H
#define IMG_H

#include "mapdata.h"

class QFile;

class IMG : public MapData
{
public:
	IMG(const QString &fileName);

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

#endif // IMG_H
