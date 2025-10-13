#ifndef PMTILE_H
#define PMTILE_H

#include <QByteArray>
#include <QBuffer>
#include <QImageReader>
#include <QPixmap>
#include "common/util.h"

class PMTile
{
public:
	PMTile(int zoom, int overzoom, int scaledSize, int style, const QPoint &xy,
	  const QByteArray &data, quint8 tc, const QString &key)
	  : _zoom(zoom), _overzoom(overzoom), _scaledSize(scaledSize),
	  _style(style), _xy(xy), _data(data), _key(key), _tc(tc) {}

	const QPoint &xy() const {return _xy;}
	const QString &key() const {return _key;}
	const QPixmap &pixmap() const {return _pixmap;}

	void load() {
		QByteArray data((_tc == 2) ? Util::gunzip(_data) : _data);
		QBuffer buffer(&data);

		if (_scaledSize) {
			QByteArray format(QByteArray::number(_zoom)
			  + ';' + QByteArray::number(_overzoom)
			  + ';' + QByteArray::number(_style));
			QImageReader reader(&buffer, format);
			reader.setScaledSize(QSize(_scaledSize, _scaledSize));
			_pixmap = QPixmap::fromImageReader(&reader);
		} else {
			QImageReader reader(&buffer);
			_pixmap = QPixmap::fromImageReader(&reader);
		}
	}

private:
	int _zoom;
	int _overzoom;
	int _scaledSize;
	int _style;
	QPoint _xy;
	QByteArray _data;
	QString _key;
	QPixmap _pixmap;
	quint8 _tc;
};

#endif // PMTILE_H
