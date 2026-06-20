#ifndef TILE_H
#define TILE_H

#include <QVariant>
#include <QPixmap>
#include <QPoint>

class FileTile
{
public:
	FileTile(const QPoint &xy, const QString &file)
	  : _xy(xy), _file(file) {}

	const QPoint &xy() const {return _xy;}
	const QString &file() const {return _file;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {_pixmap.load(_file);}

private:
	QPoint _xy;
	QString _file;
	QPixmap _pixmap;
};

class DataTile
{
public:
	DataTile(const QPoint &xy, int zoom, const QByteArray &data)
	  : _xy(xy), _zoom(zoom), _data(data) {}

	const QPoint &xy() const {return _xy;}
	const int &zoom() const {return _zoom;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {_pixmap.loadFromData(_data);}

private:
	QPoint _xy;
	int _zoom;
	QByteArray _data;
	QPixmap _pixmap;
};

#endif // TILE_H
