#include <cctype>
#include <QFile>
#include <QFileInfo>
#include "tar.h"


#define BLOCKSIZE 512

#define BLOCKCOUNT(size) \
	((size)/BLOCKSIZE + ((size) % BLOCKSIZE > 0 ? 1 : 0))

struct Header
{
	char name[100];               /*   0 */
	char mode[8];                 /* 100 */
	char uid[8];                  /* 108 */
	char gid[8];                  /* 116 */
	char size[12];                /* 124 */
	char mtime[12];               /* 136 */
	char chksum[8];               /* 148 */
	char typeflag;                /* 156 */
	char linkname[100];           /* 157 */
	char magic[6];                /* 257 */
	char version[2];              /* 263 */
	char uname[32];               /* 265 */
	char gname[32];               /* 297 */
	char devmajor[8];             /* 329 */
	char devminor[8];             /* 337 */
	char prefix[155];             /* 345 */
	                              /* 500 */
};

static quint64 number(const char* data, size_t size, int base = 8)
{
	const char *sp;
	quint64 val = 0;

	for (sp = data; sp < data + size; sp++)
		if (isdigit(*sp))
			break;
	for (; sp < data + size && isdigit(*sp); sp++)
		val = val * base + *sp - '0';

	return val;
}

bool Tar::load(const QString &path)
{
	if (_file.isOpen())
		_file.close();
	_index.clear();

	_file.setFileName(path);
	if (!_file.open(QIODevice::ReadOnly))
		return false;

	QFileInfo fi(path);
	QString tmiPath = fi.path() + "/" + fi.completeBaseName() + ".tmi";

	if (loadTmi(tmiPath))
		return true;
	else
		return loadTar();
}

bool Tar::loadTar()
{
	char buffer[BLOCKSIZE];
	struct Header *hdr = (struct Header*)&buffer;
	quint64 size;
	qint64 ret;

	while ((ret = _file.read(buffer, BLOCKSIZE)) > 0) {
		if (ret < BLOCKSIZE) {
			_file.close();
			_index.clear();
			return false;
		}
		size = number(hdr->size, sizeof(hdr->size));
		_index.insert(hdr->name, _file.pos() / BLOCKSIZE - 1);
		if (!_file.seek(_file.pos() + BLOCKCOUNT(size) * BLOCKSIZE)) {
			_file.close();
			_index.clear();
			return false;
		}
	}

	return true;
}

bool Tar::loadTmi(const QString &path)
{
	quint64 block;
	int ln = 1;

	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		return false;

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		int pos = line.indexOf(':');
		if (line.size() < 10 || pos < 7 || !line.startsWith("block")) {
			qWarning("%s:%d: syntax error\n", qPrintable(path), ln);
			_index.clear();
			return false;
		}
		block = number(line.constData() + 6, line.size() - 6, 10);
		QString file(line.mid(pos + 1).trimmed());

		_index.insert(file, block);
		ln++;
	}

	return true;
}

QByteArray Tar::file(const QString &name)
{
	char buffer[BLOCKSIZE];
	struct Header *hdr = (struct Header*)&buffer;
	quint64 size;

	QMap<QString, quint64>::const_iterator it = _index.find(name);
	if (it == _index.end())
		return QByteArray();

	Q_ASSERT(_file.isOpen());
	if (_file.seek(it.value() * BLOCKSIZE)) {
		if (_file.read(buffer, BLOCKSIZE) < BLOCKSIZE)
			return QByteArray();
		size = number(hdr->size, sizeof(hdr->size));
		return _file.read(size);
	} else
		return QByteArray();
}
