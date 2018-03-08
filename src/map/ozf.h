#ifndef OZF_H
#define OZF_H

#include <QString>
#include <QSize>
#include <QColor>
#include <QList>
#include <QVector>
#include <QFile>
#include <QPixmap>

class OZF
{
public:
	OZF(const QString &name) : _tileSize(0), _decrypt(false), _key(0),
	  _file(name) {}

	bool open();

	QString fileName() const {return _file.fileName();}
	bool isOpen() const {return _file.isOpen();}

	int zooms() const {return _zooms.size();}
	QSize size(int zoom) const;
	QPointF scale(int zoom) const;
	QSize tileSize() const {return QSize(_tileSize, _tileSize);}
	QPixmap tile(int zoom, int x, int y);

	static bool isOZF(const QString &path);

private:
	struct Zoom {
		QSize size;
		QSize dim;
		QVector<QRgb> palette;
		QVector<quint32> tiles;
	};

	template<class T> bool readValue(T &val);
	bool read(void *data, size_t size, size_t decryptSize = 0);
	bool initOZF3();
	bool initOZF2();
	bool readHeaders();
	bool readTileTable();

	quint16 _tileSize;
	bool _decrypt;
	quint8 _key;
	QList<Zoom> _zooms;
	QFile _file;
};

#endif // OZF_H
