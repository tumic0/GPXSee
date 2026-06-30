#ifndef ZIP_H
#define ZIP_H

#include <QByteArray>
#include <QHash>
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
	struct FileInfo
	{
		FileInfo(quint32 compressedSize, quint32 uncompressedSize,
		  quint32 localHeaderOffset)
		  : compressedSize(compressedSize), uncompressedSize(uncompressedSize),
		  localHeaderOffset(localHeaderOffset) {}

		quint32 compressedSize;
		quint32 uncompressedSize;
		quint32 localHeaderOffset;
	};

	static bool readHeaders(QIODevice *device, QHash<QString, FileInfo> &files);

	QIODevice *_device;
	bool _deleteDevice;
	bool _valid;
	QHash<QString, FileInfo> _files;
};

#endif // ZIP_H
