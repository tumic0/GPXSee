#ifndef ZIP_H
#define ZIP_H

#include <QByteArray>
#include <QMap>
#include <QStringList>

class QIODevice;

class Zip
{
public:
	Zip() : _device(0), _deleteDevice(false), _valid(false) {}
	Zip(const QString &path);
	Zip(QIODevice *device);
	~Zip();

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}
	QStringList files() const {return _files.keys();}
	QByteArray file(const QString &fileName) const;

	static bool isZIP(QIODevice *device);

private:
	bool readHeaders();

	QIODevice *_device;
	bool _deleteDevice;
	bool _valid;
	QString _errorString;
	QMap<QString, quint32> _files;
};

#endif // ZIP_H
