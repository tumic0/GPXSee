#ifndef TAR_H
#define TAR_H

#include <QStringList>
#include <QMap>
#include <QFile>

class Tar
{
public:
	Tar(const QString &name) : _file(name) {}

	bool open();

	QStringList files() const {return _index.keys();}
	QByteArray file(const QString &name);
	bool contains(const QString &name) const {return _index.contains(name);}

	QString fileName() const {return _file.fileName();}
	bool isOpen() const {return _file.isOpen();}

private:
	bool loadTar();
	bool loadTmi(const QString &path);

	QFile _file;
	QMap<QString, quint64> _index;
};

#endif // TAR_H
