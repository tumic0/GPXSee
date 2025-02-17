#ifndef IMG_LIGHT_H
#define IMG_LIGHT_H

#include <QVector>
#include <QDebug>

namespace IMG {

struct Light
{
	enum Color {None, Red, Green, White, Blue, Yellow, Violet, Amber};

	struct Sector
	{
	public:
		Sector() : color(None), angle(0), range(0) {}
		Sector(Color color, quint32 angle, quint32 range)
		  : color(color), angle(angle), range(range) {}

		Color color;
		quint32 angle;
		quint32 range;
	};

	Light() : color(None), range(0) {}
	Light(Color color, quint32 range) : color(color), range(range) {}
	Light(const QVector<Sector> &sectors)
	  : color(None), range(0), sectors(sectors) {}

	Color color;
	quint32 range;
	QVector<Sector> sectors;
};

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Light::Sector &sector)
{
	dbg.nospace() << "Sector(" << sector.color << ", " << sector.angle
	  << ", " << sector.range << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const IMG::Light &light)
{
	dbg.nospace() << "Light(" << light.color << ", " << light.range << ", "
	  << light.sectors << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_LIGHT_H
