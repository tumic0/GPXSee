#ifndef ZIP_H
#define ZIP_H

#include <QByteArray>
#include <QMap>
#include <QStringList>

class QIODevice;

class Zip
{
public:
	Zip(const QString &path);
	Zip(QIODevice *device);
	~Zip();

	bool isValid() const {return _valid;}
	QStringList files() const {return _files.keys();}
	QByteArray file(const QString &fileName) const;

	static bool isZIP(QIODevice *device);

private:
	QIODevice *_device;
	bool _deleteDevice;
	bool _valid;
	QMap<QString, quint32> _files;
};

#endif // ZIP_H
