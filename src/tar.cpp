#include <cctype>
#include <QFile>
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

static quint64 number(const char* data, size_t size)
{
	const char *sp;
	quint64 val = 0;

	for (sp = data; sp < data + size; sp++)
		if (isdigit(*sp))
			break;
	for (; sp < data + size && isdigit(*sp); sp++)
		val = val * 8 + *sp - '0';

	return val;
}

bool Tar::load(const QString &path)
{
	char buffer[BLOCKSIZE];
	struct Header *hdr = (struct Header*)&buffer;
	quint64 size;
	qint64 ret;

	_file.setFileName(path);
	if (!_file.open(QIODevice::ReadOnly))
		return false;

	while ((ret = _file.read(buffer, BLOCKSIZE)) > 0) {
		if (ret < BLOCKSIZE)
			return false;
		size = number(hdr->size, sizeof(hdr->size));
		if (size)
			_index.insert(hdr->name, Info(size, _file.pos()));
		if (!_file.seek(_file.pos() + BLOCKCOUNT(size) * BLOCKSIZE))
			return false;
	}

	return true;
}

QByteArray Tar::file(const QString &name)
{
	QMap<QString, Tar::Info>::const_iterator it = _index.find(name);
	if (it == _index.end())
		return QByteArray();

	if (_file.seek(it.value().offset()))
		return _file.read(it.value().size());
	else
		return QByteArray();
}
