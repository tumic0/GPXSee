#ifndef IMG_LIGHT_H
#define IMG_LIGHT_H

#include <QVector>
#include <QDebug>

namespace IMG {

class Light
{
public:
	enum Color {None, Red, Green, White, Blue, Yellow, Violet, Amber};

	class Sector
	{
	public:
		Sector() : _color(None), _angle(0), _range(0) {}
		Sector(Color color, quint32 angle, quint32 range)
		  : _color(color), _angle(angle), _range(range) {}

		Color color() const {return _color;}
		quint32 angle() const {return _angle;}
		quint32 range() const {return _range;}

	private:
		Color _color;
		quint32 _angle;
		quint32 _range;
	};

	Light() : _color(None), _range(0) {}
	Light(Color color, quint32 range) : _color(color), _range(range) {}
	Light(const QVector<Sector> &sectors)
	  : _color(None), _range(0), _sectors(sectors) {}

	Color color() const {return _color;}
	quint32 range() const {return _range;}
	const QVector<Sector> &sectors() const {return _sectors;}

private:
	Color _color;
	quint32 _range;
	QVector<Sector> _sectors;
};

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Light::Sector &sector)
{
	dbg.nospace() << "Sector(" << sector.color() << ", " << sector.angle()
	  << ", " << sector.range() << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const IMG::Light &light)
{
	dbg.nospace() << "Light(" << light.color() << ", " << light.range() << ", "
	  << light.sectors() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_LIGHT_H
