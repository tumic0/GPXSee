#include <QDir>
#include <QFileInfo>
#include "dem.h"
#include "waypoint.h"

bool Waypoint::_useDEM = false;
bool Waypoint::_show2ndElevation = false;
QHash<QString, QPixmap> Waypoint::_symbolIcons;

QPair<qreal, qreal> Waypoint::elevations() const
{
	if (_useDEM) {
		qreal dem = DEM::elevation(coordinates());
		if (!std::isnan(dem))
			return QPair<qreal, qreal>(dem, _show2ndElevation ? elevation()
			  : NAN);
		else
			return QPair<qreal, qreal>(elevation(), NAN);
	} else {
		if (hasElevation())
			return QPair<qreal, qreal>(elevation(), _show2ndElevation
			  ? DEM::elevation(coordinates()) : NAN);
		else
			return QPair<qreal, qreal>(DEM::elevation(coordinates()), NAN);
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
			  qPrintable(files.at(i).absoluteFilePath()));
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
