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
	bool loadTar();
	bool loadTmi(const QString &path);

	QFile _file;
	QMap<QString, quint64> _index;
};

#endif // TAR_H
