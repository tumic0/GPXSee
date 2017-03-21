#ifndef TAR_H
#define TAR_H

#include <QStringList>
#include <QMap>
#include <QFile>

class Tar
{
public:
	bool load(const QString &path);

	QStringList files() const {return _index.keys();}
	QByteArray file(const QString &name);

	QString fileName() const {return _file.fileName();}
	bool isOpen() const {return _file.isOpen();}

private:
	class Info
	{
	public:
		Info(quint64 size, quint64 offset) : _size(size),  _offset(offset) {}
		quint64 size() const {return _size;}
		quint64 offset() const {return _offset;}

	private:
		quint64 _size;
		quint64 _offset;
	};

	QFile _file;
	QMap<QString, Info> _index;
};

#endif // TAR_H
