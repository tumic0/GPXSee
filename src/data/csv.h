#ifndef CSV_H
#define CSV_H

#include <QIODevice>

class CSV
{
public:
	CSV(QIODevice *device, char delimiter = ',')
	  : _device(device), _delimiter(delimiter), _line(1) {}

	bool readEntry(QStringList &list);
	bool atEnd() const {return _device->atEnd();}
	int line() const {return _line;}

private:
	QIODevice *_device;
	char _delimiter;
	int _line;
};

#endif // CSV_H
