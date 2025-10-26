#ifndef MVT_SPRITES_H
#define MVT_SPRITES_H

#include <QRect>
#include <QMap>
#include <QImage>
#include <QString>
#include <QMutex>

class QJsonObject;

namespace MVT {

class Sprites
{
public:
	Sprites() {}
	Sprites(const QString &jsonFile, const QString &imageFile);

	bool isNull() const {return _img.isNull();}
	QImage icon(const QString &name, const QColor &color = Qt::black,
	  qreal size = 1.0) const;

private:
	class Sprite {
	public:
		Sprite(const QJsonObject &json);

		const QRect &rect() const {return _rect;}
		qreal pixelRatio() const {return _pixelRatio;}
		bool sdf() const {return _sdf;}

	private:
		QRect _rect;
		qreal _pixelRatio;
		bool _sdf;
	};

	QImage sprite(const Sprite &sprite, const QColor &color, qreal scale) const;

	QMap<QString, Sprite> _sprites;
	QImage _img;
};

}

#endif // MVT_SPRITES_H
