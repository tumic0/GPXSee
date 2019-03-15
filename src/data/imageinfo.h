#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <QString>
#include <QSize>

class ImageInfo
{
public:
	ImageInfo() {}
	ImageInfo(const QString &path, const QSize &size)
	  : _path(path), _size(size) {}

	const QString &path() const {return _path;}
	const QSize &size() const {return _size;}

	bool isValid() const {return _size.isValid() && !_path.isEmpty();}

private:
	QString _path;
	QSize _size;
};

#endif // IMAGEINFO_H
