#ifndef IMG_H
#define IMG_H

#include <QFile>
#include "mapdata.h"

class IMG : public MapData
{
public:
	IMG(const QString &fileName);

	QString fileName() const {return _file.fileName();}

private:
	friend class SubFile;

	int blockSize() const {return _blockSize;}
	bool readBlock(int blockNum, char *data);
	qint64 read(char *data, qint64 maxSize);
	template<class T> bool readValue(T &val);

	QFile _file;
	quint8 _key;
	int _blockSize;
};

#endif // IMG_H
