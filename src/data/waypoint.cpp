#include <QDir>
#include <QFileInfo>
#include "map/map.h"
#include "waypoint.h"

bool Waypoint::_useDEM = false;
bool Waypoint::_show2ndElevation = false;
QHash<QString, QPixmap> Waypoint::_symbolIcons;

QPair<qreal, qreal> Waypoint::elevations(Map *map) const
{
	if (_useDEM) {
		qreal dem = map->elevation(coordinates());
		if (!std::isnan(dem))
			return QPair<qreal, qreal>(dem, _show2ndElevation ? elevation()
			  : NAN);
		else
			return QPair<qreal, qreal>(elevation(), NAN);
	} else {
		if (hasElevation()) {
			qreal dem = _show2ndElevation ? map->elevation(coordinates()) : NAN;
			return QPair<qreal, qreal>(elevation(), dem);
		} else
			return QPair<qreal, qreal>(map->elevation(coordinates()), NAN);
	}
}

void Waypoint::loadSymbolIcons(const QString &dir)
{
	if (dir.isEmpty())
		return;

	QDir d(dir);
	QFileInfoList files(d.entryInfoList(QDir::Files | QDir::Readable));

	for (int i = 0; i < files.size(); i++) {
		QPixmap pm(files.at(i).absoluteFilePath());
		if (pm.isNull())
			qWarning("%s: error loading image",
			  qUtf8Printable(files.at(i).absoluteFilePath()));
		else
			_symbolIcons.insert(files.at(i).baseName(), pm);
	}
}

const QPixmap *Waypoint::symbolIcon(const QString &symbol)
{
	if (symbol.isEmpty())
		return 0;
	QHash<QString, QPixmap>::const_iterator it(_symbolIcons.find(symbol));
	return (it == _symbolIcons.constEnd()) ? 0 : &*it;
}
