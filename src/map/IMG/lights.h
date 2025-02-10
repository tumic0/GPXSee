#ifndef IMG_LIGHTS_H
#define IMG_LIGHTS_H

#include <QVector>
#include <QDebug>

namespace IMG {

class Lights
{
public:
	enum Color {None, Red, Green, White, Blue, Yellow, Violet, Amber};

	struct Sector
	{
		Sector() : color(None), angle(0), range(0) {}
		Sector(Color color, quint32 angle, quint32 range)
		  : color(color), angle(angle), range(range) {}

		Color color;
		quint32 angle;
		quint32 range;
	};

	Lights() : color(None), range(0) {}
	bool isSectorLight() const {return ((color && range) || !sectors.isEmpty());}

	Color color;
	quint32 range;
	QVector<Sector> sectors;
};

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Lights::Sector &sector)
{
	dbg.nospace() << "Sector(" << sector.color << ", " << sector.angle
	  << ", " << sector.range << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const IMG::Lights &lights)
{
	dbg.nospace() << "Lights(" << lights.color << ", " << lights.range << ", "
	  << lights.sectors << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_LIGHTS_H
